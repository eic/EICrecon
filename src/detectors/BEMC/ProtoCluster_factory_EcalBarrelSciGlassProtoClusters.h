// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <random>

#include "extensions/jana/JChainFactoryT.h"
#include "services/geometry/dd4hep/JDD4hep_service.h"
#include "algorithms/calorimetry/CalorimeterIslandCluster.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"

class ProtoCluster_factory_EcalBarrelSciGlassProtoClusters : public JChainFactoryT<edm4eic::ProtoCluster>, CalorimeterIslandCluster {

public:
    //------------------------------------------
    // Constructor
    ProtoCluster_factory_EcalBarrelSciGlassProtoClusters(std::vector<std::string> default_input_tags)
    : JChainFactoryT<edm4eic::ProtoCluster>(std::move(default_input_tags)) {
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        InitDataTags(GetPluginName() + ":" + GetTag());

        auto app = GetApplication();

        // adjacency matrix
        m_geoSvcName = "GeoSvc";
        // Magic constants:
        //  24 - number of sectors
        //  5  - number of towers per sector
        // The following compares distance of 1 to a taxicab metric on a cylinder that is our calorimeter:
        u_adjacencyMatrix =
          "("
          "  abs(tower_1 - tower_2)"
          "  + min("
          "      abs((sector_1 - sector_2) * 5 + row_1 - row_2),"
          "      24 * 5 - abs((sector_1 - sector_2) * 5 + row_1 - row_2)"
          "  )"
          ") == 1";
        u_adjacencyMatrix.erase(
          std::remove_if(u_adjacencyMatrix.begin(), u_adjacencyMatrix.end(), ::isspace),
          u_adjacencyMatrix.end());
        m_readout = "EcalBarrelSciGlassHits";

        // neighbour checking distances
        m_sectorDist=0.0 * dd4hep::cm;             // not applicable
        u_localDistXY={};     //{this, "localDistXY", {}};
        u_localDistXZ={};     //{this, "localDistXZ", {}};
        u_localDistYZ={};     //{this, "localDistYZ", {}};
        u_globalDistRPhi={};  //{this, "globalDistRPhi", {}};
        u_globalDistEtaPhi={}; //{this, "globalDistEtaPhi", {}};
        u_dimScaledLocalDistXY={}; // not used

        m_splitCluster = true;
        m_minClusterHitEdep=1.0 * dd4hep::MeV;
        m_minClusterCenterEdep=30.0 * dd4hep::MeV;
        u_transverseEnergyProfileMetric = "globalDistEtaPhi";
        u_transverseEnergyProfileScale = 0.06;

        app->SetDefaultParameter("BEMC:EcalBarrelSciGlassProtoClusters:geoServiceName", m_geoSvcName);
        app->SetDefaultParameter("BEMC:EcalBarrelSciGlassProtoClusters:readoutClass", m_readout);
        app->SetDefaultParameter("BEMC:EcalBarrelSciGlassProtoClusters:sectorDist",   m_sectorDist);
        app->SetDefaultParameter("BEMC:EcalBarrelSciGlassProtoClusters:localDistXY",   u_localDistXY);
        app->SetDefaultParameter("BEMC:EcalBarrelSciGlassProtoClusters:localDistXZ",   u_localDistXZ);
        app->SetDefaultParameter("BEMC:EcalBarrelSciGlassProtoClusters:localDistYZ",  u_localDistYZ);
        app->SetDefaultParameter("BEMC:EcalBarrelSciGlassProtoClusters:globalDistRPhi",    u_globalDistRPhi);
        app->SetDefaultParameter("BEMC:EcalBarrelSciGlassProtoClusters:globalDistEtaPhi",    u_globalDistEtaPhi);
        app->SetDefaultParameter("BEMC:EcalBarrelSciGlassProtoClusters:dimScaledLocalDistXY",    u_dimScaledLocalDistXY);
        app->SetDefaultParameter("BEMC:EcalBarrelSciGlassProtoClusters:adjacencyMatrix", u_adjacencyMatrix);
        app->SetDefaultParameter("BEMC:EcalBarrelSciGlassProtoClusters:splitCluster", m_splitCluster);
        app->SetDefaultParameter("BEMC:EcalBarrelSciGlassProtoClusters:minClusterHitEdep", m_minClusterHitEdep);
        app->SetDefaultParameter("BEMC:EcalBarrelSciGlassProtoClusters:minClusterCenterEdep", m_minClusterCenterEdep);
        app->SetDefaultParameter("BEMC:EcalBarrelSciGlassProtoClusters:transverseEnergyProfileMetric", u_transverseEnergyProfileMetric);
        app->SetDefaultParameter("BEMC:EcalBarrelSciGlassProtoClusters:transverseEnergyProfileScale", u_transverseEnergyProfileScale);
        m_geoSvc = app->template GetService<JDD4hep_service>();

        AlgorithmInit(m_log);
    }

    //------------------------------------------
    // ChangeRun
    void ChangeRun(const std::shared_ptr<const JEvent> &event) override{
        AlgorithmChangeRun();
    }

    //------------------------------------------
    // Process
    void Process(const std::shared_ptr<const JEvent> &event) override{
        // Prefill inputs
        hits = event->Get<edm4eic::CalorimeterHit>(GetInputTags()[0]);

        // Call Process for generic algorithm
        AlgorithmProcess();

        // Hand owner of algorithm objects over to JANA
        Set(protoClusters);
        protoClusters.clear(); // not really needed, but better to not leave dangling pointers around
    }
};
