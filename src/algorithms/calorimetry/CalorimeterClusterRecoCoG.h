
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

    edm4eic::Cluster* reconstruct(const edm4eic::ProtoCluster* pcl) const;

};
