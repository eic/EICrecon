
// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten, Chao, Chao Peng, Whitney Armstrong

/*
 *  Reconstruct the cluster with Center of Gravity method
 *  Logarithmic weighting is used for mimicing energy deposit in transverse direction
 *
 *  Author: Chao Peng (ANL), 09/27/2020
 */

#pragma once

#include <random>

#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <edm4hep/SimCalorimeterHit.h>
#include <edm4eic/ProtoCluster.h>
#include <edm4eic/Cluster.h>

#include <edm4eic/MCRecoClusterParticleAssociation.h>
#include <edm4eic/MutableMCRecoClusterParticleAssociation.h>
#include <edm4eic/MutableCluster.h>
#include <edm4eic/vector_utils.h>
#include <map>
#include <spdlog/spdlog.h>



static double constWeight(double /*E*/, double /*tE*/, double /*p*/, int /*type*/) { return 1.0; }
    static double linearWeight(double E, double /*tE*/, double /*p*/, int /*type*/) { return E; }
    static double logWeight(double E, double tE, double base, int /*type*/) {
      return std::max(0., base + std::log(E / tE));
    }

    static const std::map<std::string, std::function<double(double, double, double, int)>> weightMethods={
      {"none", constWeight},
      {"linear", linearWeight},
      {"log", logWeight},
    };

class CalorimeterClusterRecoCoG {

    // Insert any member variables here

public:
    CalorimeterClusterRecoCoG() = default;
    ~CalorimeterClusterRecoCoG(){} // better to use smart pointer?
    virtual void AlgorithmInit(std::shared_ptr<spdlog::logger>& logger);
    virtual void AlgorithmChangeRun() ;
    virtual void AlgorithmProcess() ;

    //-------- Configuration Parameters ------------
    //instantiate new spdlog logger
    std::shared_ptr<spdlog::logger> m_log;


    std::string m_input_simhit_tag;
    std::string m_input_protoclust_tag;

    double m_sampFrac         = 1.;//{this, "samplingFraction", 1.0};
    double m_logWeightBase = 3.6;
    double m_depthCorrection  = 0.;//{this, "depthCorrection", 0.0};
    std::string m_energyWeight;//{this, "energyWeight", "log"};
    std::string m_moduleDimZName;//{this, "moduleDimZName", ""};
    // Constrain the cluster position eta to be within
    // the eta of the contributing hits. This is useful to avoid edge effects
    // for endcaps.
    bool m_enableEtaBounds    = false;//{this, "enableEtaBounds", false};

    std::shared_ptr<JDD4hep_service> m_geoSvc;

    std::function<double(double, double, double, int)> weightFunc;


  //inputs EcalEndcapNTruthProtoClusters AND EcalEndcapNHits

  //inputs
    std::vector<const edm4hep::SimCalorimeterHit*> m_inputSimhits; //e.g. EcalEndcapNHits
    std::vector<const edm4eic::ProtoCluster*> m_inputProto; //e.g. EcalEndcapNTruthProtoClusters  //{"outputProtoClusters", Gaudi::DataHandle::Writer, this};

  //outputs
    std::vector<edm4eic::Cluster*> m_outputClusters;
    std::vector<edm4eic::MCRecoClusterParticleAssociation*> m_outputAssociations;


private:
edm4eic::Cluster* reconstruct(const edm4eic::ProtoCluster* pcl) const {
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

    // center of gravity with logarithmic weighting
    float tw = 0.;
    float timeWeighted = 0.;
    float timeErrorWeighted = 0.;
    auto v   = cl.getPosition();
    for (unsigned i = 0; i < pcl->getHits().size(); ++i) {
      const auto& hit   = pcl->getHits()[i];
      const auto weight = pcl->getWeights()[i];
//      _DBG_<<" -- weight = " << weight << "  E=" << hit.getEnergy() << " totalE=" <<totalE << " log(E/totalE)=" << std::log(hit.getEnergy()/totalE) << std::endl;
      float w           = weightFunc(hit.getEnergy() * weight, totalE, m_logWeightBase, 0);
      tw += w;
      v = v + (hit.getPosition() * w);
      timeWeighted = timeWeighted + hit.getTime() * w;
      timeErrorWeighted = timeErrorWeighted + hit.getTimeError() * w;
    }

    if (tw == 0.) {
      m_log->warn("zero total weights encountered, you may want to adjust your weighting parameter.");
    }
    cl.setPosition(v / tw);
    cl.setTime(time / tw);
    cl.setTimeError(timeError / tw);

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

    // Calculate radius
    // @TODO: add skewness
    if (cl.getNhits() > 1) {
      double radius = 0;
      for (const auto& hit : pcl->getHits()) {
        const auto delta = cl.getPosition() - hit.getPosition();
        radius += delta * delta;
      }
      radius = sqrt((1. / (cl.getNhits() - 1.)) * radius);
      cl.addToShapeParameters(radius);
      cl.addToShapeParameters(0 /* skewness */); // skewness not yet calculated
    }
//    edm4eic::Cluster c(cl);
    return new edm4eic::Cluster(cl);
  }


};
