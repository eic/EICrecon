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

class ProtoCluster_factory_HcalEndcapPInsertIslandProtoClusters : public JFactoryT<edm4eic::ProtoCluster>, CalorimeterIslandCluster {

public:
    //------------------------------------------
    // Constructor
    ProtoCluster_factory_HcalEndcapPInsertIslandProtoClusters(){
        SetTag("HcalEndcapPInsertIslandProtoClusters");
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();
        m_input_tag = "HcalEndcapPInsertMergedHits";

        m_splitCluster=true;              // https://eicweb.phy.anl.gov/EIC/juggler/-/blob/main/JugReco/src/components/CalorimeterIslandCluster.cpp
        m_minClusterHitEdep=0.0 * MeV;    // https://eicweb.phy.anl.gov/EIC/juggler/-/blob/main/JugReco/src/components/CalorimeterIslandCluster.cpp
        m_minClusterCenterEdep=30.0 * MeV; // from ATHENA's reconstruction.py

        // neighbour checking distances
        m_sectorDist=5.0 * cm;             // https://eicweb.phy.anl.gov/EIC/juggler/-/blob/main/JugReco/src/components/CalorimeterIslandCluster.cpp
        u_localDistXY={};     //{this, "localDistXY", {}};
        u_localDistXZ={};     //{this, "localDistXZ", {}};
        u_localDistYZ={};     //{this, "localDistYZ", {}};
        u_globalDistRPhi={};  //{this, "globalDistRPhi", {}};
        u_globalDistEtaPhi={};//{this, "globalDistEtaPhi", {}};
        u_dimScaledLocalDistXY={15.0*dd4hep::mm, 15.0*dd4hep::mm};// from ATHENA's reconstruction.py


        app->SetDefaultParameter("HCAL:HcalEndcapPInsertIslandProtoClusters:splitCluster",             m_splitCluster);
        app->SetDefaultParameter("HCAL:HcalEndcapPInsertIslandProtoClusters:minClusterHitEdep",  m_minClusterHitEdep);
        app->SetDefaultParameter("HCAL:HcalEndcapPInsertIslandProtoClusters:minClusterCenterEdep",     m_minClusterCenterEdep);
        app->SetDefaultParameter("HCAL:HcalEndcapPInsertIslandProtoClusters:sectorDist",   m_sectorDist);
        app->SetDefaultParameter("HCAL:HcalEndcapPInsertIslandProtoClusters:localDistXY",   u_localDistXY);
        app->SetDefaultParameter("HCAL:HcalEndcapPInsertIslandProtoClusters:localDistXZ",   u_localDistXZ);
        app->SetDefaultParameter("HCAL:HcalEndcapPInsertIslandProtoClusters:localDistYZ",  u_localDistYZ);
        app->SetDefaultParameter("HCAL:HcalEndcapPInsertIslandProtoClusters:globalDistRPhi",    u_globalDistRPhi);
        app->SetDefaultParameter("HCAL:HcalEndcapPInsertIslandProtoClusters:globalDistEtaPhi",    u_globalDistEtaPhi);
        app->SetDefaultParameter("HCAL:HcalEndcapPInsertIslandProtoClusters:dimScaledLocalDistXY",    u_dimScaledLocalDistXY);
        m_geoSvc = app->template GetService<JDD4hep_service>();

        std::string tag=this->GetTag();
        std::shared_ptr<spdlog::logger> m_log = app->GetService<Log_service>()->logger(tag);

        // Get log level from user parameter or default
        std::string log_level_str = "info";
        auto pm = app->GetJParameterManager();
        pm->SetDefaultParameter(tag + ":LogLevel", log_level_str, "verbosity: trace, debug, info, warn, err, critical, off");
        m_log->set_level(eicrecon::ParseLogLevel(log_level_str));
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

