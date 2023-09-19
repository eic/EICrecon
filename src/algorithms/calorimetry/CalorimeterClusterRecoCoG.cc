// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten, Chao, Chao Peng, Whitney Armstrong, Dhevan Gangadharan

#include <Evaluator/DD4hepUnits.h>
/*
 *  Reconstruct the cluster with Center of Gravity method
 *  Logarithmic weighting is used for mimicking energy deposit in transverse direction
 *
 *  Author: Chao Peng (ANL), 09/27/2020
 */
#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptor/map.hpp>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/vector_utils_legacy.h>
#include <edm4hep/CaloHitContributionCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <fmt/core.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <Eigen/Core>
#include <Eigen/Eigenvalues>
#include <Eigen/Householder>
#include <Eigen/Jacobi>
#include <cctype>
#include <complex>
#include <exception>
#include <limits>
#include <map>
#include <new>
#include <type_traits>
#include <vector>

#include "CalorimeterClusterRecoCoG.h"
#include "algorithms/calorimetry/CalorimeterClusterRecoCoGConfig.h"

namespace eicrecon {

  using namespace dd4hep;

  void CalorimeterClusterRecoCoG::init(const dd4hep::Detector* detector, std::shared_ptr<spdlog::logger>& logger) {
    m_log = logger;
    m_detector = detector;

    // update depth correction if a name is provided
    if (!m_cfg.moduleDimZName.empty()) {
      m_cfg.depthCorrection = m_detector->constantAsDouble(m_cfg.moduleDimZName);
    }

    // select weighting method
    std::string ew = m_cfg.energyWeight;
    // make it case-insensitive
    std::transform(ew.begin(), ew.end(), ew.begin(), [](char s) { return std::tolower(s); });
    auto it = weightMethods.find(ew);
    if (it == weightMethods.end()) {
      m_log->error("Cannot find energy weighting method {}, choose one from [{}]", m_cfg.energyWeight, boost::algorithm::join(weightMethods | boost::adaptors::map_keys, ", "));
      return;
    }
    weightFunc = it->second;
  }

  ClustersWithAssociations CalorimeterClusterRecoCoG::process(
            const edm4eic::ProtoClusterCollection* proto,
            const edm4hep::SimCalorimeterHitCollection* mchits) {

    // output collections
    auto clusters = std::make_unique<edm4eic::ClusterCollection>();
    auto associations = std::make_unique<edm4eic::MCRecoClusterParticleAssociationCollection>();

    for (const auto& pcl : *proto) {

      // skip protoclusters with no hits
      if (pcl.hits_size() == 0) {
        continue;
      }

      auto cl = reconstruct(pcl);

      m_log->debug("{} hits: {} GeV, ({}, {}, {})", cl.getNhits(), cl.getEnergy() / dd4hep::GeV, cl.getPosition().x / dd4hep::mm, cl.getPosition().y / dd4hep::mm, cl.getPosition().z / dd4hep::mm);
      clusters->push_back(cl);

      // If mcHits are available, associate cluster with MCParticle
      // 1. find proto-cluster hit with largest energy deposition
      // 2. find first mchit with same CellID
      // 3. assign mchit's MCParticle as cluster truth
      if (mchits->size() > 0) {

        // 1. find pclhit with largest energy deposition
        auto pclhits = pcl.getHits();
        auto pclhit = std::max_element(
          pclhits.begin(),
          pclhits.end(),
          [](const auto& pclhit1, const auto& pclhit2) {
            return pclhit1.getEnergy() < pclhit2.getEnergy();
          }
        );

        // FIXME: The code below fails for HcalEndcapPClusters. This does not happen for
        // FIXME: all calorimeters. A brief scan of the code suggests this could be caused
        // FIXME: by the CalorimeterHitDigi algorithm modifying the cellID for the raw hits.
        // FIXME: Thus, the cellID values passed on through to here no longer match those
        // FIXME: in the low-level truth hits. It likely works for other detectors because
        // FIXME: their u_fields and u_refs members are left empty which effectively results
        // FIXME: in the cellID being unchanged.

        // 2. find mchit with same CellID
        // find_if not working, https://github.com/AIDASoft/podio/pull/273
        //auto mchit = std::find_if(
        //  mchits->begin(),
        //  mchits->end(),
        //  [&pclhit](const auto& mchit1) {
        //    return mchit1.getCellID() == pclhit->getCellID();
        //  }
        //);
        auto mchit = mchits->begin();
        for ( ; mchit != mchits->end(); ++mchit) {
          // break loop when CellID match found
          if ( mchit->getCellID() == pclhit->getCellID()) {
            break;
          }
        }
        if (!(mchit != mchits->end())) {
          // break if no matching hit found for this CellID
          m_log->warn("Proto-cluster has highest energy in CellID {}, but no mc hit with that CellID was found.", pclhit->getCellID());
          m_log->trace("Proto-cluster hits: ");
          for (const auto& pclhit1: pclhits) {
            m_log->trace("{}: {}", pclhit1.getCellID(), pclhit1.getEnergy());
          }
          m_log->trace("MC hits: ");
          for (const auto& mchit1: *mchits) {
            m_log->trace("{}: {}", mchit1.getCellID(), mchit1.getEnergy());
          }
          break;
        }

        // 3. find mchit's MCParticle
        const auto& mcp = mchit->getContributions(0).getParticle();

        m_log->debug("cluster has largest energy in cellID: {}", pclhit->getCellID());
        m_log->debug("pcl hit with highest energy {} at index {}", pclhit->getEnergy(), pclhit->getObjectID().index);
        m_log->debug("corresponding mc hit energy {} at index {}", mchit->getEnergy(), mchit->getObjectID().index);
        m_log->debug("from MCParticle index {}, PDG {}, {}", mcp.getObjectID().index, mcp.getPDG(), edm4eic::magnitude(mcp.getMomentum()));

        // set association
        auto clusterassoc = associations->create();
        clusterassoc.setRecID(cl.getObjectID().index); // if not using collection, this is always set to -1
        clusterassoc.setSimID(mcp.getObjectID().index);
        clusterassoc.setWeight(1.0);
        clusterassoc.setRec(cl);
        clusterassoc.setSim(mcp);
      } else {
        m_log->debug("No mcHitCollection was provided, so no truth association will be performed.");
      }
    }

    return std::make_pair(std::move(clusters), std::move(associations));
}

//------------------------------------------------------------------------
edm4eic::Cluster CalorimeterClusterRecoCoG::reconstruct(const edm4eic::ProtoCluster& pcl) const {
  edm4eic::MutableCluster cl;
  cl.setNhits(pcl.hits_size());

  m_log->debug("hit size = {}", pcl.hits_size());

  // no hits
  if (pcl.hits_size() == 0) {
    return cl;
  }

  // calculate total energy, find the cell with the maximum energy deposit
  float totalE = 0.;
  float maxE   = 0.;
  // Used to optionally constrain the cluster eta to those of the contributing hits
  float minHitEta = std::numeric_limits<float>::max();
  float maxHitEta = std::numeric_limits<float>::min();
  auto time       = pcl.getHits()[0].getTime();
  auto timeError  = pcl.getHits()[0].getTimeError();
  for (unsigned i = 0; i < pcl.getHits().size(); ++i) {
    const auto& hit   = pcl.getHits()[i];
    const auto weight = pcl.getWeights()[i];
    m_log->debug("hit energy = {} hit weight: {}", hit.getEnergy(), weight);
    auto energy = hit.getEnergy() * weight;
    totalE += energy;
    if (energy > maxE) {
    }
    const float eta = edm4eic::eta(hit.getPosition());
    if (eta < minHitEta) {
      minHitEta = eta;
    }
    if (eta > maxHitEta) {
      maxHitEta = eta;
    }
  }
  cl.setEnergy(totalE / m_cfg.sampFrac);
  cl.setEnergyError(0.);
  cl.setTime(time);
  cl.setTimeError(timeError);

  // center of gravity with logarithmic weighting
  float tw = 0.;
  auto v   = cl.getPosition();
  for (unsigned i = 0; i < pcl.getHits().size(); ++i) {
    const auto& hit   = pcl.getHits()[i];
    const auto weight = pcl.getWeights()[i];
    //      _DBG_<<" -- weight = " << weight << "  E=" << hit.getEnergy() << " totalE=" <<totalE << " log(E/totalE)=" << std::log(hit.getEnergy()/totalE) << std::endl;
    float w           = weightFunc(hit.getEnergy() * weight, totalE, m_cfg.logWeightBase, 0);
    tw += w;
    v = v + (hit.getPosition() * w);
  }
  if (tw == 0.) {
    m_log->warn("zero total weights encountered, you may want to adjust your weighting parameter.");
  }
  cl.setPosition(v / tw);
  cl.setPositionError({}); // @TODO: Covariance matrix

  // Optionally constrain the cluster to the hit eta values
  if (m_cfg.enableEtaBounds) {
    const bool overflow  = (edm4eic::eta(cl.getPosition()) > maxHitEta);
    const bool underflow = (edm4eic::eta(cl.getPosition()) < minHitEta);
    if (overflow || underflow) {
      const double newEta   = overflow ? maxHitEta : minHitEta;
      const double newTheta = edm4eic::etaToAngle(newEta);
      const double newR     = edm4eic::magnitude(cl.getPosition());
      const double newPhi   = edm4eic::angleAzimuthal(cl.getPosition());
      cl.setPosition(edm4eic::sphericalToVector(newR, newTheta, newPhi));
      m_log->debug("Bound cluster position to contributing hits due to {}", (overflow ? "overflow" : "underflow"));
    }
  }

  // Additional convenience variables

  // best estimate on the cluster direction is the cluster position
  // for simple 2D CoG clustering
  cl.setIntrinsicTheta(edm4eic::anglePolar(cl.getPosition()));
  cl.setIntrinsicPhi(edm4eic::angleAzimuthal(cl.getPosition()));
  // TODO errors

  //_______________________________________
  // Calculate cluster profile:
  //    radius,
  //    dispersion (energy weighted radius),
  //    theta-phi cluster widths (2D)
  //    x-y-z cluster widths (3D)
  float radius = 0, dispersion = 0, w_sum = 0;

  Eigen::Matrix2f sum2_2D = Eigen::Matrix2f::Zero();
  Eigen::Matrix3f sum2_3D = Eigen::Matrix3f::Zero();
  Eigen::Vector2f sum1_2D = Eigen::Vector2f::Zero();
  Eigen::Vector3f sum1_3D = Eigen::Vector3f::Zero();
  Eigen::Vector2cf eigenValues_2D = Eigen::Vector2cf::Zero();
  Eigen::Vector3cf eigenValues_3D = Eigen::Vector3cf::Zero();

  if (cl.getNhits() > 1) {

    for (const auto& hit : pcl.getHits()) {

      float w = weightFunc(hit.getEnergy(), cl.getEnergy(), m_cfg.logWeightBase, 0);

      // theta, phi
      Eigen::Vector2f pos2D( edm4eic::anglePolar( hit.getPosition() ), edm4eic::angleAzimuthal( hit.getPosition() ) );
      // x, y, z
      Eigen::Vector3f pos3D( hit.getPosition().x, hit.getPosition().y, hit.getPosition().z );

      const auto delta = cl.getPosition() - hit.getPosition();
      radius          += delta * delta;
      dispersion      += delta * delta * w;

      // Weighted Sum x*x, x*y, x*z, y*y, etc.
      sum2_2D += w * pos2D * pos2D.transpose();
      sum2_3D += w * pos3D * pos3D.transpose();

      // Weighted Sum x, y, z
      sum1_2D += w * pos2D;
      sum1_3D += w * pos3D;

      w_sum += w;
    }

    if( w_sum > 0 ) {
      radius     = sqrt((1. / (cl.getNhits() - 1.)) * radius);
      dispersion = sqrt( dispersion / w_sum );

      // normalize matrices
      sum2_2D /= w_sum;
      sum2_3D /= w_sum;
      sum1_2D /= w_sum;
      sum1_3D /= w_sum;

      // 2D and 3D covariance matrices
      Eigen::Matrix2f cov2 = sum2_2D - sum1_2D * sum1_2D.transpose();
      Eigen::Matrix3f cov3 = sum2_3D - sum1_3D * sum1_3D.transpose();

      // Solve for eigenvalues.  Corresponds to cluster's 2nd moments (widths)
      Eigen::EigenSolver<Eigen::Matrix2f> es_2D(cov2, false); // set to true for eigenvector calculation
      Eigen::EigenSolver<Eigen::Matrix3f> es_3D(cov3, false); // set to true for eigenvector calculation

      // eigenvalues of symmetric real matrix are always real
      eigenValues_2D = es_2D.eigenvalues();
      eigenValues_3D = es_3D.eigenvalues();
    }
  }

  cl.addToShapeParameters( radius );
  cl.addToShapeParameters( dispersion );
  cl.addToShapeParameters( eigenValues_2D[0].real() ); // 2D theta-phi cluster width 1
  cl.addToShapeParameters( eigenValues_2D[1].real() ); // 2D theta-phi cluster width 2
  cl.addToShapeParameters( eigenValues_3D[0].real() ); // 3D x-y-z cluster width 1
  cl.addToShapeParameters( eigenValues_3D[1].real() ); // 3D x-y-z cluster width 2
  cl.addToShapeParameters( eigenValues_3D[2].real() ); // 3D x-y-z cluster width 3

  return std::move(cl);
}

} // eicrecon
