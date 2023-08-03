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

class ProtoCluster_factory_EcalBarrelScFiProtoClusters : public JChainFactoryT<edm4eic::ProtoCluster>, CalorimeterIslandCluster {

public:
    //------------------------------------------
    // Constructor
    ProtoCluster_factory_EcalBarrelScFiProtoClusters(std::vector<std::string> default_input_tags)
    : JChainFactoryT<edm4eic::ProtoCluster>(std::move(default_input_tags)) {
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        InitDataTags(GetPluginName() + ":" + GetTag());

        auto app = GetApplication();

        m_splitCluster=false;               // from ATHENA reconstruction.py
        m_minClusterHitEdep=1.0 * dd4hep::MeV;    // from ATHENA reconstruction.py
        m_minClusterCenterEdep=10.0 * dd4hep::MeV; // from ATHENA reconstruction.py

        // adjacency matrix
        m_geoSvcName = "GeoSvc";
        u_adjacencyMatrix = "";
        u_adjacencyMatrix.erase(
          std::remove_if(u_adjacencyMatrix.begin(), u_adjacencyMatrix.end(), ::isspace),
          u_adjacencyMatrix.end());
        m_readout = "";

        // neighbour checking distances
        m_sectorDist=50. * dd4hep::mm;             // connects hits from different sectors
        u_localDistXY={};     //{this, "localDistXY", {}};
        u_localDistXZ={40 * dd4hep::mm, 40 * dd4hep::mm};     //{this, "localDistXZ", {}};  n.b. 30 * dd4hep::mm, 30 * dd4hep::mm came from comment Maria Z. put into PR.
        u_localDistYZ={};     //{this, "localDistYZ", {}};
        u_globalDistRPhi={};  //{this, "globalDistRPhi", {}};
        u_globalDistEtaPhi={};//{this, "globalDistEtaPhi", {}};
        u_dimScaledLocalDistXY={};// from ATHENA reconstruction.py

        app->SetDefaultParameter("BEMC:EcalBarrelScFiProtoClusters:splitCluster",             m_splitCluster);
        app->SetDefaultParameter("BEMC:EcalBarrelScFiProtoClusters:minClusterHitEdep",  m_minClusterHitEdep);
        app->SetDefaultParameter("BEMC:EcalBarrelScFiProtoClusters:minClusterCenterEdep",     m_minClusterCenterEdep);
        app->SetDefaultParameter("BEMC:EcalBarrelScFiProtoClusters:sectorDist",   m_sectorDist);
        app->SetDefaultParameter("BEMC:EcalBarrelScFiProtoClusters:localDistXY",   u_localDistXY);
        app->SetDefaultParameter("BEMC:EcalBarrelScFiProtoClusters:localDistXZ",   u_localDistXZ);
        app->SetDefaultParameter("BEMC:EcalBarrelScFiProtoClusters:localDistYZ",  u_localDistYZ);
        app->SetDefaultParameter("BEMC:EcalBarrelScFiProtoClusters:globalDistRPhi",    u_globalDistRPhi);
        app->SetDefaultParameter("BEMC:EcalBarrelScFiProtoClusters:globalDistEtaPhi",    u_globalDistEtaPhi);
        app->SetDefaultParameter("BEMC:EcalBarrelScFiProtoClusters:dimScaledLocalDistXY",    u_dimScaledLocalDistXY);
        app->SetDefaultParameter("BEMC:EcalBarrelScFiProtoClusters:adjacencyMatrix", u_adjacencyMatrix);
        app->SetDefaultParameter("BEMC:EcalBarrelScFiProtoClusters:geoServiceName", m_geoSvcName);
        app->SetDefaultParameter("BEMC:EcalBarrelScFiProtoClusters:readoutClass", m_readout);
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
