// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten, Chao, Chao Peng, Whitney Armstrong

/*
 *  Reconstruct the cluster with Center of Gravity method
 *  Logarithmic weighting is used for mimicing energy deposit in transverse direction
 *
 *  Author: Chao Peng (ANL), 09/27/2020
            Dhevan Gangadharan (UH): cluster profiling from Eigenvalues
 */
#include "CalorimeterClusterRecoCoG.h"

#include <JANA/JEvent.h>
#include <Evaluator/DD4hepUnits.h>
#include <fmt/format.h>
#include <map>

#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptor/map.hpp>
#include <edm4hep/MCParticle.h>


using namespace dd4hep;

//------------------------
// AlgorithmInit
//------------------------

void CalorimeterClusterRecoCoG::AlgorithmInit(std::shared_ptr<spdlog::logger>& logger) {

    m_log=logger;
    // update depth correction if a name is provided
    if (m_moduleDimZName != "") {
      m_depthCorrection = m_geoSvc->detector()->constantAsDouble(m_moduleDimZName);
    }

    // select weighting method
    std::string ew = m_energyWeight;
    // make it case-insensitive
    std::transform(ew.begin(), ew.end(), ew.begin(), [](char s) { return std::tolower(s); });
    auto it = weightMethods.find(ew);
    if (it == weightMethods.end()) {
      m_log->error("Cannot find energy weighting method {}, choose one from [{}]", m_energyWeight, boost::algorithm::join(weightMethods | boost::adaptors::map_keys, ", "));
      return;
    }
    weightFunc = it->second;
    // info() << "z_length " << depth << endmsg;

    return;
}

//------------------------
// AlgorithmChangeRun
//------------------------
void CalorimeterClusterRecoCoG::AlgorithmChangeRun() {
}

//------------------------
// AlgorithmProcess
//------------------------
void CalorimeterClusterRecoCoG::AlgorithmProcess() {

    // input collections
    const auto& proto  = m_inputProto;
    auto& clusters     = m_outputClusters;

    // Optional input MC data
    std::vector<const edm4hep::SimCalorimeterHit*> mchits = m_inputSimhits;

    // Optional output associations
    //associations removed in favor of referencing underlying vector m_outputAssociations
    //std::vector<edm4eic::MCRecoClusterParticleAssociation*> associations = m_outputAssociations;


    for (const auto& pcl : proto) {
      auto cl = reconstruct(pcl);

      // skip null clusters
      if (cl == nullptr) continue;

      m_log->debug("{} hits: {} GeV, ({}, {}, {})", cl->getNhits(), cl->getEnergy() / dd4hep::GeV, cl->getPosition().x / dd4hep::mm, cl->getPosition().y / dd4hep::mm, cl->getPosition().z / dd4hep::mm);
      clusters.push_back(cl);

      // If mcHits are available, associate cluster with MCParticle
      // 1. find proto-cluster hit with largest energy deposition
      // 2. find first mchit with same CellID
      // 3. assign mchit's MCParticle as cluster truth
//      if (!mchits.empty() && !m_outputAssociations.empty()) {  // ? having m_outputAssociations be not empty doesn't make sense ?
      if (!mchits.empty() ) {

        // 1. find pclhit with largest energy deposition
        auto pclhits = pcl->getHits();
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
        //  mchits.begin(),
        //  mchits.end(),
        //  [&pclhit](const auto& mchit1) {
        //    return mchit1.getCellID() == pclhit->getCellID();
        //  }
        //);
        auto mchit = mchits.begin();
        for ( ; mchit != mchits.end(); ++mchit) {
          // break loop when CellID match found
          if ( (*mchit)->getCellID() == pclhit->getCellID()) {
            break;
          }
        }
        if (!(mchit != mchits.end())) {
          // break if no matching hit found for this CellID
          m_log->warn("Proto-cluster has highest energy in CellID {}, but no mc hit with that CellID was found.", pclhit->getCellID());
          m_log->trace("Proto-cluster hits: ");
          for (const auto& pclhit1: pclhits) {
            m_log->trace("{}: {}", pclhit1.getCellID(), pclhit1.getEnergy());
          }
          m_log->trace("MC hits: ");
          for (const auto& mchit1: mchits) {
            m_log->trace("{}: {}", mchit1->getCellID(), mchit1->getEnergy());
          }
          break;
        }

        // 3. find mchit's MCParticle
        const auto& mcp = (*mchit)->getContributions(0).getParticle();

        m_log->debug("cluster has largest energy in cellID: {}", pclhit->getCellID());
        m_log->debug("pcl hit with highest energy {} at index {}", pclhit->getEnergy(), pclhit->getObjectID().index);
        m_log->debug("corresponding mc hit energy {} at index {}", (*mchit)->getEnergy(), (*mchit)->getObjectID().index);
        m_log->debug("from MCParticle index {}, PDG {}, {}", mcp.getObjectID().index, mcp.getPDG(), edm4eic::magnitude(mcp.getMomentum()));

        // set association
        edm4eic::MutableMCRecoClusterParticleAssociation* clusterassoc = new edm4eic::MutableMCRecoClusterParticleAssociation();
//        clusterassoc->setRecID(cl->getObjectID().index); // if not using collection, this is always set to -1
        clusterassoc->setRecID((uint32_t)((uint64_t)cl&0xFFFFFFFF)); // mask lower 32 bits of cluster pointer as unique ID FIXME:
        clusterassoc->setSimID(mcp.getObjectID().index);
        clusterassoc->setWeight(1.0);
        clusterassoc->setRec(*cl);
        //clusterassoc.setSim(mcp);
        edm4eic::MCRecoClusterParticleAssociation* cassoc = new edm4eic::MCRecoClusterParticleAssociation(*clusterassoc);
        m_outputAssociations.push_back(cassoc);
        delete clusterassoc;
      } else {
        m_log->debug("No mcHitCollection was provided, so no truth association will be performed.");
      }
    }

    return;
}

//------------------------------------------------------------------------
edm4eic::Cluster* CalorimeterClusterRecoCoG::reconstruct(const edm4eic::ProtoCluster* pcl) const {
  edm4eic::MutableCluster cl;
  cl.setNhits(pcl->hits_size());

  m_log->debug("hit size = {}", pcl->hits_size());

  // no hits
  if (pcl->hits_size() == 0) {
    return nullptr;
  }

  // calculate total energy, find the cell with the maximum energy deposit
  float totalE = 0.;
  float maxE   = 0.;
  // Used to optionally constrain the cluster eta to those of the contributing hits
  float minHitEta = std::numeric_limits<float>::max();
  float maxHitEta = std::numeric_limits<float>::min();
  auto time       = pcl->getHits()[0].getTime();
  auto timeError  = pcl->getHits()[0].getTimeError();
  for (unsigned i = 0; i < pcl->getHits().size(); ++i) {
    const auto& hit   = pcl->getHits()[i];
    const auto weight = pcl->getWeights()[i];
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
  cl.setEnergy(totalE / m_sampFrac);
  cl.setEnergyError(0.);
  cl.setTime(time);
  cl.setTimeError(timeError);

  // center of gravity with logarithmic weighting
  float tw = 0.;
  auto v   = cl.getPosition();
  for (unsigned i = 0; i < pcl->getHits().size(); ++i) {
    const auto& hit   = pcl->getHits()[i];
    const auto weight = pcl->getWeights()[i];
    //      _DBG_<<" -- weight = " << weight << "  E=" << hit.getEnergy() << " totalE=" <<totalE << " log(E/totalE)=" << std::log(hit.getEnergy()/totalE) << std::endl;
    float w           = weightFunc(hit.getEnergy() * weight, totalE, m_logWeightBase, 0);
    tw += w;
    v = v + (hit.getPosition() * w);
  }
  if (tw == 0.) {
    m_log->warn("zero total weights encountered, you may want to adjust your weighting parameter.");
  }
  cl.setPosition(v / tw);
  cl.setPositionError({}); // @TODO: Covariance matrix

  // Optionally constrain the cluster to the hit eta values
  if (m_enableEtaBounds) {
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
  //   	dispersion (energy weighted radius),
  //   	sigma_long
  //   	sigma_short
  //   	sigma_z
  double radius = 0, dispersion = 0, lambda_1 = 0, lambda_2 = 0, lambda_3 = 0;
  double w_sum = 0;
  double sum_11 = 0, sum_22 = 0, sum_33 = 0;
  double sum_12 = 0, sum_13 = 0, sum_23 = 0;
  double sum_1 = 0, sum_2 = 0, sum_3 = 0;

  if (cl.getNhits() > 1) {

    for (const auto& hit : pcl->getHits()) {

      const auto delta = cl.getPosition() - hit.getPosition();
      radius += delta * delta;

      double w = weightFunc(hit.getEnergy(), cl.getEnergy(), m_logWeightBase, 0);
      w_sum += w;
      dispersion += delta * delta * w;

      double pos_1 = edm4eic::anglePolar( hit.getPosition() );
      double pos_2 = edm4eic::angleAzimuthal( hit.getPosition() );
      double pos_3 = hit.getPosition().z;

      if( m_xyClusterProfiling ) {
        pos_1 = hit.getPosition().x;
        pos_2 = hit.getPosition().y;
      }
      sum_11 += w * pos_1 * pos_1;
      sum_22 += w * pos_2 * pos_2;
      sum_33 += w * pos_3 * pos_3;
      sum_12 += w * pos_1 * pos_2;
      sum_13 += w * pos_1 * pos_3;
      sum_23 += w * pos_2 * pos_3;
      sum_1  += w * pos_1; 
      sum_2  += w * pos_2;
      sum_3  += w * pos_3;
    }

    if( w_sum > 0 ) {
      radius = sqrt((1. / (cl.getNhits() - 1.)) * radius);
      dispersion = sqrt( dispersion / w_sum );

      // variances and covariances 
      double sigma_11 = sum_11 / w_sum - (sum_1/w_sum) * (sum_1/w_sum);
      double sigma_22 = sum_22 / w_sum - (sum_2/w_sum) * (sum_2/w_sum);
      double sigma_33 = sum_33 / w_sum - (sum_3/w_sum) * (sum_3/w_sum);
      double sigma_12 = sum_12 / w_sum - (sum_1/w_sum) * (sum_2/w_sum);
      double sigma_13 = sum_13 / w_sum - (sum_1/w_sum) * (sum_3/w_sum);
      double sigma_23 = sum_23 / w_sum - (sum_2/w_sum) * (sum_3/w_sum);

      // covariance matrix
      Eigen::MatrixXd cov3(3,3);
      cov3(0,0) = sigma_11;  cov3(1,1) = sigma_22;  cov3(2,2) = sigma_33;
      cov3(0,1) = sigma_12;  cov3(0,2) = sigma_13;  cov3(1,2) = sigma_23;
      cov3(1,0) = cov3(0,1); cov3(2,0) = cov3(0,2); cov3(2,1) = cov3(1,2);

      // Eigenvalues correspond to cluster's 2nd moments (sigma_long, sigma_short, sigma_z)
      Eigen::EigenSolver<Eigen::MatrixXd> es(cov3, false); // set to true for eigenvector calculation
      auto eigenValues = es.eigenvalues();
      lambda_1 = eigenValues[0].real(); // imaginary parts correspond to unphysical roots
      lambda_2 = eigenValues[1].real();
      lambda_3 = eigenValues[2].real();
    }
  } 

  cl.addToShapeParameters( radius );
  cl.addToShapeParameters( dispersion );
  cl.addToShapeParameters( lambda_1 );
  cl.addToShapeParameters( lambda_2 );
  cl.addToShapeParameters( lambda_3 );

  return new edm4eic::Cluster(cl);
}
