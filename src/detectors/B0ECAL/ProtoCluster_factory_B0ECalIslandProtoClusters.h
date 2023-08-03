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

class ProtoCluster_factory_B0ECalIslandProtoClusters : public JChainFactoryT<edm4eic::ProtoCluster>, CalorimeterIslandCluster {

public:
    //------------------------------------------
    // Constructor
    ProtoCluster_factory_B0ECalIslandProtoClusters(std::vector<std::string> default_input_tags)
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
        u_adjacencyMatrix = "";
        u_adjacencyMatrix.erase(
          std::remove_if(u_adjacencyMatrix.begin(), u_adjacencyMatrix.end(), ::isspace),
          u_adjacencyMatrix.end());
        m_readout = "";

        // neighbour checking distances
        m_sectorDist=5.0 * dd4hep::cm;             // from ATHENA reconstruction.py
        u_localDistXY={};     //{this, "localDistXY", {}};
        u_localDistXZ={};     //{this, "localDistXZ", {}};
        u_localDistYZ={};     //{this, "localDistYZ", {}};
        u_globalDistRPhi={};  //{this, "globalDistRPhi", {}};
        u_globalDistEtaPhi={};//{this, "globalDistEtaPhi", {}};
        u_dimScaledLocalDistXY={1.8,1.8};// from ATHENA reconstruction.py

        m_splitCluster=false;
        m_minClusterHitEdep=1.0 * dd4hep::MeV;
        m_minClusterCenterEdep=30.0 * dd4hep::MeV;
        u_transverseEnergyProfileMetric = "globalDistEtaPhi";
        u_transverseEnergyProfileScale = 1.;

        app->SetDefaultParameter("B0ECAL:B0ECalIslandProtoClusters:geoServiceName", m_geoSvcName);
        app->SetDefaultParameter("B0ECAL:B0ECalIslandProtoClusters:readoutClass", m_readout);
        app->SetDefaultParameter("B0ECAL:B0ECalIslandProtoClusters:sectorDist",   m_sectorDist);
        app->SetDefaultParameter("B0ECAL:B0ECalIslandProtoClusters:localDistXY",   u_localDistXY);
        app->SetDefaultParameter("B0ECAL:B0ECalIslandProtoClusters:localDistXZ",   u_localDistXZ);
        app->SetDefaultParameter("B0ECAL:B0ECalIslandProtoClusters:localDistYZ",  u_localDistYZ);
        app->SetDefaultParameter("B0ECAL:B0ECalIslandProtoClusters:globalDistRPhi",    u_globalDistRPhi);
        app->SetDefaultParameter("B0ECAL:B0ECalIslandProtoClusters:globalDistEtaPhi",    u_globalDistEtaPhi);
        app->SetDefaultParameter("B0ECAL:B0ECalIslandProtoClusters:dimScaledLocalDistXY",    u_dimScaledLocalDistXY);
        app->SetDefaultParameter("B0ECAL:B0ECalIslandProtoClusters:adjacencyMatrix", u_adjacencyMatrix);
        app->SetDefaultParameter("B0ECAL:B0ECalIslandProtoClusters:splitCluster", m_splitCluster);
        app->SetDefaultParameter("B0ECAL:B0ECalIslandProtoClusters:minClusterHitEdep", m_minClusterHitEdep);
        app->SetDefaultParameter("B0ECAL:B0ECalIslandProtoClusters:minClusterCenterEdep", m_minClusterCenterEdep);
        app->SetDefaultParameter("B0ECAL:B0ECalIslandProtoClusters:transverseEnergyProfileMetric", u_transverseEnergyProfileMetric);
        app->SetDefaultParameter("B0ECAL:B0ECalIslandProtoClusters:transverseEnergyProfileScale", u_transverseEnergyProfileScale);
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
        // Get input collection
        auto hits_coll = static_cast<const edm4eic::CalorimeterHitCollection*>(event->GetCollectionBase(GetInputTags()[0]));

        // Call Process for generic algorithm
        auto protoclust_coll = AlgorithmProcess(*hits_coll);

        // Hand algorithm objects over to JANA
        SetCollection(std::move(protoclust_coll));
    }
};
