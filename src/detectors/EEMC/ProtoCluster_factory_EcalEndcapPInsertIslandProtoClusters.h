// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <random>

#include <JANA/JFactoryT.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/calorimetry/CalorimeterIslandCluster.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>

class ProtoCluster_factory_EcalEndcapPInsertIslandProtoClusters : public JFactoryT<edm4eic::ProtoCluster>, CalorimeterIslandCluster {

public:
    //------------------------------------------
    // Constructor
    ProtoCluster_factory_EcalEndcapPInsertIslandProtoClusters(){
        SetTag("EcalEndcapPInsertIslandProtoClusters");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();
        m_input_tag = "EcalEndcapPInsertRecHits";

        m_splitCluster=false;               // from ATHENA reconstruction.py
        m_minClusterHitEdep=1.0 * dd4hep::MeV;    // from ATHENA reconstruction.py
        m_minClusterCenterEdep=30.0 * dd4hep::MeV; // from ATHENA reconstruction.py

        // neighbour checking distances
        m_sectorDist=5.0 * dd4hep::cm;             // from ATHENA reconstruction.py
        u_localDistXY={};     //{this, "localDistXY", {}};
        u_localDistXZ={};     //{this, "localDistXZ", {}};
        u_localDistYZ={};     //{this, "localDistYZ", {}};
        u_globalDistRPhi={};  //{this, "globalDistRPhi", {}};
        u_globalDistEtaPhi={};//{this, "globalDistEtaPhi", {}};
        u_dimScaledLocalDistXY={1.8,1.8};// from ATHENA reconstruction.py


        app->SetDefaultParameter("EEMC:EcalEndcapPInsertClusters:input_tag",        m_input_tag, "Name of input collection to use");
        app->SetDefaultParameter("EEMC:EcalEndcapPInsertClusters:splitCluster",             m_splitCluster);
        app->SetDefaultParameter("EEMC:EcalEndcapPInsertClusters:minClusterHitEdep",  m_minClusterHitEdep);
        app->SetDefaultParameter("EEMC:EcalEndcapPInsertClusters:minClusterCenterEdep",     m_minClusterCenterEdep);
        app->SetDefaultParameter("EEMC:EcalEndcapPInsertClusters:sectorDist",   m_sectorDist);
        app->SetDefaultParameter("EEMC:EcalEndcapPInsertClusters:localDistXY",   u_localDistXY);
        app->SetDefaultParameter("EEMC:EcalEndcapPInsertClusters:localDistXZ",   u_localDistXZ);
        app->SetDefaultParameter("EEMC:EcalEndcapPInsertClusters:localDistYZ",  u_localDistYZ);
        app->SetDefaultParameter("EEMC:EcalEndcapPInsertClusters:globalDistRPhi",    u_globalDistRPhi);
        app->SetDefaultParameter("EEMC:EcalEndcapPInsertClusters:globalDistEtaPhi",    u_globalDistEtaPhi);
        app->SetDefaultParameter("EEMC:EcalEndcapPInsertClusters:dimScaledLocalDistXY",    u_dimScaledLocalDistXY);
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

