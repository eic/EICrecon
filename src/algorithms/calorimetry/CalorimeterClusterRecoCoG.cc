// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten, Chao, Chao Peng, Whitney Armstrong, Dhevan Gangadharan

/*
 *  Reconstruct the cluster with Center of Gravity method
 *  Logarithmic weighting is used for mimicking energy deposit in transverse direction
 *
 *  Author: Chao Peng (ANL), 09/27/2020
 */

#include <Evaluator/DD4hepUnits.h>
#include <DD4hep/Readout.h>
#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptor/map.hpp>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4hep/CaloHitContributionCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <Eigen/Core>
#include <Eigen/Eigenvalues>
#include <Eigen/Householder> // IWYU pragma: keep
#include <TString.h>
#include <cctype>
#include <complex>
#include <cstddef>
#include <gsl/pointers>
#include <limits>
#include <map>
#include <optional>
#include <vector>

#include "CalorimeterClusterRecoCoG.h"
#include "algorithms/calorimetry/CalorimeterClusterRecoCoGConfig.h"

namespace eicrecon {

  using namespace dd4hep;

  void CalorimeterClusterRecoCoG::init(const dd4hep::Detector* detector, const dd4hep::rec::CellIDPositionConverter* converter) {

    // set services needed for associations
    m_detector = detector;
    m_converter = converter;

    // select weighting method
    std::string ew = m_cfg.energyWeight;
    // make it case-insensitive
    std::transform(ew.begin(), ew.end(), ew.begin(), [](char s) { return std::tolower(s); });
    auto it = weightMethods.find(ew);
    if (it == weightMethods.end()) {
      error("Cannot find energy weighting method {}, choose one from [{}]", m_cfg.energyWeight, boost::algorithm::join(weightMethods | boost::adaptors::map_keys, ", "));
      return;
    }
    weightFunc = it->second;
  }

  void CalorimeterClusterRecoCoG::process(
      const CalorimeterClusterRecoCoG::Input& input,
      const CalorimeterClusterRecoCoG::Output& output) const {

    const auto [proto, mchits] = input;
    auto [clusters, associations] = output;

    for (const auto& pcl : *proto) {

      // skip protoclusters with no hits
      if (pcl.hits_size() == 0) {
        continue;
      }

      auto cl_opt = reconstruct(pcl);
      if (! cl_opt.has_value()) {
        continue;
      }
      auto cl = *std::move(cl_opt);

      debug("{} hits: {} GeV, ({}, {}, {})", cl.getNhits(), cl.getEnergy() / dd4hep::GeV, cl.getPosition().x / dd4hep::mm, cl.getPosition().y / dd4hep::mm, cl.getPosition().z / dd4hep::mm);
      clusters->push_back(cl);

      // If mcHits are available, do truth association
      if (mchits->size() > 0) {
        associate(cl, pcl, std::move(mchits), std::move(associations));
      } else {
        debug("No mcHitCollection was provided, so no truth association will be performed.");
      }
    }
}

//------------------------------------------------------------------------
std::optional<edm4eic::Cluster> CalorimeterClusterRecoCoG::reconstruct(const edm4eic::ProtoCluster& pcl) const {
  edm4eic::MutableCluster cl;
  cl.setNhits(pcl.hits_size());

  debug("hit size = {}", pcl.hits_size());

  // no hits
  if (pcl.hits_size() == 0) {
    return {};
  }

  // calculate total energy, find the cell with the maximum energy deposit
  float totalE = 0.;
  // Used to optionally constrain the cluster eta to those of the contributing hits
  float minHitEta = std::numeric_limits<float>::max();
  float maxHitEta = std::numeric_limits<float>::min();
  auto time       = pcl.getHits()[0].getTime();
  auto timeError  = pcl.getHits()[0].getTimeError();
  for (unsigned i = 0; i < pcl.getHits().size(); ++i) {
    const auto& hit   = pcl.getHits()[i];
    const auto weight = pcl.getWeights()[i];
    debug("hit energy = {} hit weight: {}", hit.getEnergy(), weight);
    auto energy = hit.getEnergy() * weight;
    totalE += energy;
    cl.addToHits(hit);
    cl.addToHitContributions(energy);
    const float eta = edm4hep::utils::eta(hit.getPosition());
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

  double logWeightBase=m_cfg.logWeightBase;
  if (m_cfg.logWeightBaseCoeffs.size() != 0){
    double l=log(cl.getEnergy()/m_cfg.logWeightBase_Eref);
    logWeightBase=0;
    for(std::size_t i =0; i<m_cfg.logWeightBaseCoeffs.size(); i++){
      logWeightBase += m_cfg.logWeightBaseCoeffs[i]*pow(l,i);
    }
  }

  for (unsigned i = 0; i < pcl.getHits().size(); ++i) {
    const auto& hit   = pcl.getHits()[i];
    const auto weight = pcl.getWeights()[i];
    //      _DBG_<<" -- weight = " << weight << "  E=" << hit.getEnergy() << " totalE=" <<totalE << " log(E/totalE)=" << std::log(hit.getEnergy()/totalE) << std::endl;
    float w           = weightFunc(hit.getEnergy() * weight, totalE, logWeightBase, 0);
    tw += w;
    v = v + (hit.getPosition() * w);
  }
  if (tw == 0.) {
    warning("zero total weights encountered, you may want to adjust your weighting parameter.");
    return {};
  }
  cl.setPosition(v / tw);
  cl.setPositionError({}); // @TODO: Covariance matrix

  // Optionally constrain the cluster to the hit eta values
  if (m_cfg.enableEtaBounds) {
    const bool overflow  = (edm4hep::utils::eta(cl.getPosition()) > maxHitEta);
    const bool underflow = (edm4hep::utils::eta(cl.getPosition()) < minHitEta);
    if (overflow || underflow) {
      const double newEta   = overflow ? maxHitEta : minHitEta;
      const double newTheta = edm4hep::utils::etaToAngle(newEta);
      const double newR     = edm4hep::utils::magnitude(cl.getPosition());
      const double newPhi   = edm4hep::utils::angleAzimuthal(cl.getPosition());
      cl.setPosition(edm4hep::utils::sphericalToVector(newR, newTheta, newPhi));
      debug("Bound cluster position to contributing hits due to {}", (overflow ? "overflow" : "underflow"));
    }
  }

  // Additional convenience variables

  // best estimate on the cluster direction is the cluster position
  // for simple 2D CoG clustering
  cl.setIntrinsicTheta(edm4hep::utils::anglePolar(cl.getPosition()));
  cl.setIntrinsicPhi(edm4hep::utils::angleAzimuthal(cl.getPosition()));
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
      Eigen::Vector2f pos2D( edm4hep::utils::anglePolar( hit.getPosition() ), edm4hep::utils::angleAzimuthal( hit.getPosition() ) );
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

//------------------------------------------------------------------------
void CalorimeterClusterRecoCoG::associate(
  const edm4eic::Cluster& cl,
  const edm4eic::ProtoCluster& pcl,
  const edm4hep::SimCalorimeterHitCollection* mchits,
  edm4eic::MCRecoClusterParticleAssociationCollection* assocs
) const {

  // associate each cluster to all contributing
  // shower initiators
  //   1. for each protocluster hit, get list of contributing particles
  //   2. identify each contributor whose start vertex is *outside*
  //      the calorimeter but whose stop vertex is *inside* the
  //      calorimeter [TODO: would it be better to just check
  //      the start vertex? That would also catch particles that
  //      MIP through...]
  //   3. assign that particle
  for (auto protoHit : pcl.getHits()) {

    // identify corresponding mc hit
    std::optional<edm4hep::SimCalorimeterHit> protoSimHit;
    for (
      auto mcHit = mchits->begin();
      mcHit != mchits->end();
      ++mcHit
    ) {
      if (mcHit->getCellID() == protoHit.getCellID()) {
        protoSimHit = *mcHit;
        break;
      }
    }  // end mc hit loop

    // if no sim hit found, continue
    if (!protoSimHit.has_value()) {
      warning("No mc hit with that CellID {} found for protocluster hit!", protoHit.getCellID());
      continue;
    }

    // loop over contributing particles
    for (auto contrib : protoSimHit.value().getContributions()) {

      // grab particle and calculate weight
      auto mcPar = contrib.getParticle();
      double weight = contrib.getEnergy() / cl.getEnergy();

      // get start/stop vertices
      edm4hep::Vector3d start = mcPar.getVertex();
      edm4hep::Vector3d stop  = mcPar.getEndpoint();

      // check if either is in detector
      const bool isStartInDet = isVtxInDet(start, protoHit);
      const bool isStopInDet  = isVtxInDet(stop, protoHit);

      // fill association iff
      //   - particle started outside detector, and
      //   - particle stops inside detector
      if (!isStartInDet && isStopInDet) {
        auto clusterassoc = assocs->create();
        clusterassoc.setRecID(cl.getObjectID().index); // if not using collection, this is always set to -1
        clusterassoc.setSimID(mcPar.getObjectID().index);
        clusterassoc.setWeight(weight);
        clusterassoc.setRec(cl);
        clusterassoc.setSim(mcPar);
      }
    }  // end contribution loop
  }  // end protocluster hit loop
}  // end 'associate(edm4eic::Cluster& cl, edm4eic::ProtoCluster&, edm4hep::SimCalorimeterHitCollection*, edm4eic::MCRecoClusterParticleAssociation*)'

//------------------------------------------------------------------------
//  - FIXME use std::string's rather than TStrings
//  - TODO clean up: only need to grab constants once per process
bool CalorimeterClusterRecoCoG::isVtxInDet(const edm4hep::Vector3d& vertex, const edm4eic::CalorimeterHit& hit) const {

  // get readout
  auto readout = m_converter -> findReadout(
    m_converter -> findDetElement(
      m_converter -> position(hit.getCellID())
    )
  );

  // get detector name from readout
  TString sDetName(readout.name());
  sDetName.ReplaceAll("Hits", "_");

  // grab min/max r/z for detector
  double rMin = std::numeric_limits<double>::min();
  double rMax = std::numeric_limits<double>::max();
  double zMin = -1. * std::numeric_limits<double>::max();
  double zMax = std::numeric_limits<double>::max();
  for (auto constant : m_detector -> constants()) {

    // only consider constants from this detector
    //   - FIXME certain detectors break this scheme
    //     Maybe there's a better way?
    TString sConstant(constant.first.data());
    if (!sConstant.Contains(sDetName.Data())) continue;

    // set rmin
    if (sConstant.Contains("rmin")) {
      rMin = m_detector -> constant<double>(constant.first);
    }

    // set rmax
    if (sConstant.Contains("rmax")) {
      rMax = m_detector -> constant<double>(constant.first);
    }

    // set zmin
    if (sConstant.Contains("zmin")) {
      zMin = m_detector -> constant<double>(constant.first);
    }

    // set zmax
    if (sConstant.Contains("zmax")) {
      zMax = m_detector -> constant<double>(constant.first);
    }
  }  // end constant loop

  // check if vtx is in r/z range and return
  const double zVtx  = vertex.z;
  const double rVtx  = std::hypot(vertex.x, vertex.y);
  const bool   isInZ = ((zVtx >= zMin) && (zVtx <= zMax));
  const bool   isInR = ((rVtx >= rMin) && (rVtx <= rMax));
  return (isInZ && isInR);

}  // end 'IsVtxInDet(edm4hep::Vector3d&, edm4eic::CalorimeterHit&)'

} // eicrecon
