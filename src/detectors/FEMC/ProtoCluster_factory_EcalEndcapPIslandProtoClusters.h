// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <random>

#include <services/io/podio/JFactoryPodioT.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/calorimetry/CalorimeterIslandCluster.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>

class ProtoCluster_factory_EcalEndcapPIslandProtoClusters : public eicrecon::JFactoryPodioT<edm4eic::ProtoCluster>, CalorimeterIslandCluster {

public:
    //------------------------------------------
    // Constructor
    ProtoCluster_factory_EcalEndcapPIslandProtoClusters(){
        SetTag("EcalEndcapPIslandProtoClusters");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();
        m_input_tag = "EcalEndcapPRecHits";

        m_splitCluster=false;               // from ATHENA reconstruction.py
        m_minClusterHitEdep=0.0 * dd4hep::MeV;    // from ATHENA reconstruction.py
        m_minClusterCenterEdep=10.0 * dd4hep::MeV; // from ATHENA reconstruction.py

        // adjacency matrix
        m_geoSvcName = "GeoSvc";
        u_adjacencyMatrix = "";
        u_adjacencyMatrix.erase(
          std::remove_if(u_adjacencyMatrix.begin(), u_adjacencyMatrix.end(), ::isspace),
          u_adjacencyMatrix.end());
        m_readout = "";

        // neighbour checking distances
        m_sectorDist=5.0 * dd4hep::cm;             // from ATHENA reconstruction.py
        u_localDistXY={10.0 * dd4hep::cm, 10.0 * dd4hep::cm};     //{this, "localDistXY", {}};
        u_localDistXZ={};     //{this, "localDistXZ", {}};
        u_localDistYZ={};     //{this, "localDistYZ", {}};
        u_globalDistRPhi={};  //{this, "globalDistRPhi", {}};
        u_globalDistEtaPhi={};//{this, "globalDistEtaPhi", {}};
        u_dimScaledLocalDistXY={1.5,1.5};// from ATHENA reconstruction.py


        app->SetDefaultParameter("FEMC:EcalEndcapPIslandProtoClusters:input_tag",        m_input_tag, "Name of input collection to use");
        app->SetDefaultParameter("FEMC:EcalEndcapPIslandProtoClusters:splitCluster",             m_splitCluster);
        app->SetDefaultParameter("FEMC:EcalEndcapPIslandProtoClusters:minClusterHitEdep",  m_minClusterHitEdep);
        app->SetDefaultParameter("FEMC:EcalEndcapPIslandProtoClusters:minClusterCenterEdep",     m_minClusterCenterEdep);
        app->SetDefaultParameter("FEMC:EcalEndcapPIslandProtoClusters:sectorDist",   m_sectorDist);
        app->SetDefaultParameter("FEMC:EcalEndcapPIslandProtoClusters:localDistXY",   u_localDistXY);
        app->SetDefaultParameter("FEMC:EcalEndcapPIslandProtoClusters:localDistXZ",   u_localDistXZ);
        app->SetDefaultParameter("FEMC:EcalEndcapPIslandProtoClusters:localDistYZ",  u_localDistYZ);
        app->SetDefaultParameter("FEMC:EcalEndcapPIslandProtoClusters:globalDistRPhi",    u_globalDistRPhi);
        app->SetDefaultParameter("FEMC:EcalEndcapPIslandProtoClusters:globalDistEtaPhi",    u_globalDistEtaPhi);
        app->SetDefaultParameter("FEMC:EcalEndcapPIslandProtoClusters:dimScaledLocalDistXY",    u_dimScaledLocalDistXY);
        app->SetDefaultParameter("FEMC:EcalEndcapPIslandProtoClusters:adjacencyMatrix", u_adjacencyMatrix);
        app->SetDefaultParameter("FEMC:EcalEndcapPIslandProtoClusters:geoServiceName", m_geoSvcName);
        app->SetDefaultParameter("FEMC:EcalEndcapPIslandProtoClusters:readoutClass", m_readout);
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
        hits = event->Get<edm4eic::CalorimeterHit>(m_input_tag);

        // Call Process for generic algorithm
        AlgorithmProcess();

        // Hand owner of algorithm objects over to JANA
        Set(protoClusters);
        protoClusters.clear(); // not really needed, but better to not leave dangling pointers around
    }
};
