// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef _ProtoCLuster_factory_EcalEndcapNIslandProtoClusters_h_
#define _ProtoCLuster_factory_EcalEndcapNIslandProtoClusters_h_

#include <random>

#include <JANA/JFactoryT.h>

#include <edm4eic/ProtoClusterCollection.h>

#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/calorimetry/CalorimeterIslandCluster.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>
#include <extensions/jana/JPodioUtils.h>

class ProtoCluster_factory_EcalEndcapNIslandProtoClusters : public JFactoryT<edm4eic::ProtoCluster>, CalorimeterIslandCluster {

public:
    //------------------------------------------
    // Constructor
    ProtoCluster_factory_EcalEndcapNIslandProtoClusters(){
        SetTag("EcalEndcapNIslandProtoClusters");
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();
        m_input_tag = "EcalEndcapNRecHits";

        m_splitCluster=false;               // from ATHENA reconstruction.py
        m_minClusterHitEdep=1.0 * MeV;    // from ATHENA reconstruction.py
        m_minClusterCenterEdep=30.0 * MeV; // from ATHENA reconstruction.py

        // neighbour checking distances
        m_sectorDist=5.0 * cm;             // from ATHENA reconstruction.py
        u_localDistXY={};     //{this, "localDistXY", {}};
        u_localDistXZ={};     //{this, "localDistXZ", {}};
        u_localDistYZ={};     //{this, "localDistYZ", {}};
        u_globalDistRPhi={};  //{this, "globalDistRPhi", {}};
        u_globalDistEtaPhi={};//{this, "globalDistEtaPhi", {}};
        u_dimScaledLocalDistXY={1.8,1.8};// from ATHENA reconstruction.py


        app->SetDefaultParameter("EEMC:splitCluster",             m_splitCluster);
        app->SetDefaultParameter("EEMC:minClusterHitEdep",  m_minClusterHitEdep);
        app->SetDefaultParameter("EEMC:minClusterCenterEdep",     m_minClusterCenterEdep);
        //app->SetDefaultParameter("EEMC:inputHitCollection", m_inputHitCollection);
        //app->SetDefaultParameter("EEMC:outputProtoClusterCollection",    m_outputProtoCollection);
        app->SetDefaultParameter("EEMC:sectorDist",   m_sectorDist);
        app->SetDefaultParameter("EEMC:localDistXY",   u_localDistXY);
        app->SetDefaultParameter("EEMC:localDistXZ",   u_localDistXZ);
        app->SetDefaultParameter("EEMC:localDistYZ",  u_localDistYZ);
        app->SetDefaultParameter("EEMC:globalDistRPhi",    u_globalDistRPhi);
        app->SetDefaultParameter("EEMC:globalDistEtaPhi",    u_globalDistEtaPhi);
        app->SetDefaultParameter("EEMC:dimScaledLocalDistXY",    u_dimScaledLocalDistXY);
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
        StorePodioData<edm4eic::ProtoCluster, edm4eic::ProtoClusterCollection>(protoClusters, this, event);
        protoClusters.clear(); // not really needed, but better to not leave dangling pointers around
    }
};

#endif // _ProtoCLuster_factory_EcalEndcapNIslandProtoClusters_h_
