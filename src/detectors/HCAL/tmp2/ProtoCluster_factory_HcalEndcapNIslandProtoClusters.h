// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef _ProtoCLuster_factory_HcalEndcapNIslandProtoClusters_h_
#define _ProtoCLuster_factory_HcalEndcapNIslandProtoClusters_h_

#include <random>

#include <JANA/JFactoryT.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/calorimetry/CalorimeterIslandCluster.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>

class ProtoCluster_factory_HcalEndcapNIslandProtoClusters : public JFactoryT<edm4eic::ProtoCluster>, CalorimeterIslandCluster {

public:
    //------------------------------------------
    // Constructor
    ProtoCluster_factory_HcalEndcapNIslandProtoClusters(){
        SetTag("HcalEndcapNIslandProtoClusters");
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();
        m_input_tag = "HcalEndcapNRecHits";

        m_splitCluster=true;              // from https://eicweb.phy.anl.gov/EIC/detectors/athena/-/blob/master/calibrations/ffi_zdc.json
        m_minClusterHitEdep=0.1 * MeV;    // from https://eicweb.phy.anl.gov/EIC/detectors/athena/-/blob/master/calibrations/ffi_zdc.json
        m_minClusterCenterEdep=3.0 * MeV; // from https://eicweb.phy.anl.gov/EIC/detectors/athena/-/blob/master/calibrations/ffi_zdc.json

        // neighbour checking distances
        m_sectorDist=5.0 * cm;             // from ATHENA reconstruction.py
        u_localDistXY={};     //{this, "localDistXY", {}};
        u_localDistXZ={};     //{this, "localDistXZ", {}};
        u_localDistYZ={};     //{this, "localDistYZ", {}};
        u_globalDistRPhi={};  //{this, "globalDistRPhi", {}};
        u_globalDistEtaPhi={};//{this, "globalDistEtaPhi", {}};
        u_dimScaledLocalDistXY={50.0*dd4hep::mm, 50.0*dd4hep::mm};// from https://eicweb.phy.anl.gov/EIC/detectors/athena/-/blob/master/calibrations/ffi_zdc.json


        app->SetDefaultParameter("ZDC:splitCluster",             m_splitCluster);
        app->SetDefaultParameter("ZDC:minClusterHitEdep",  m_minClusterHitEdep);
        app->SetDefaultParameter("ZDC:minClusterCenterEdep",     m_minClusterCenterEdep);
        //app->SetDefaultParameter("ZDC:inputHitCollection", m_inputHitCollection);
        //app->SetDefaultParameter("ZDC:outputProtoClusterCollection",    m_outputProtoCollection);
        app->SetDefaultParameter("ZDC:sectorDist",   m_sectorDist);
        app->SetDefaultParameter("ZDC:localDistXY",   u_localDistXY);
        app->SetDefaultParameter("ZDC:localDistXZ",   u_localDistXZ);
        app->SetDefaultParameter("ZDC:localDistYZ",  u_localDistYZ);
        app->SetDefaultParameter("ZDC:globalDistRPhi",    u_globalDistRPhi);
        app->SetDefaultParameter("ZDC:globalDistEtaPhi",    u_globalDistEtaPhi);
        app->SetDefaultParameter("ZDC:dimScaledLocalDistXY",    u_dimScaledLocalDistXY);
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

#endif // _ProtoCLuster_factory_HcalEndcapNIslandProtoClusters_h_
