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

class ProtoCluster_factory_ZDCEcalIslandProtoClusters : public eicrecon::JFactoryPodioT<edm4eic::ProtoCluster>, CalorimeterIslandCluster {

public:
    //------------------------------------------
    // Constructor
    ProtoCluster_factory_ZDCEcalIslandProtoClusters(){
        SetTag("ZDCEcalIslandProtoClusters");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();
        m_input_tag = "ZDCEcalRecHits";

        m_splitCluster=true;              // from https://eicweb.phy.anl.gov/EIC/detectors/athena/-/blob/master/calibrations/ffi_zdc.json
        m_minClusterHitEdep=0.1 * dd4hep::MeV;    // from https://eicweb.phy.anl.gov/EIC/detectors/athena/-/blob/master/calibrations/ffi_zdc.json
        m_minClusterCenterEdep=3.0 * dd4hep::MeV; // from https://eicweb.phy.anl.gov/EIC/detectors/athena/-/blob/master/calibrations/ffi_zdc.json

        // adjacency matrix
        m_geoSvcName = "GeoSvc";
        u_adjacencyMatrix = "";
        u_adjacencyMatrix.erase(
          std::remove_if(u_adjacencyMatrix.begin(), u_adjacencyMatrix.end(), ::isspace),
          u_adjacencyMatrix.end());
        m_readout = "";

        // neighbour checking distances
        m_sectorDist=5.0 * dd4hep::cm;             // from ATHENA reconstruction.py
        u_localDistXY={50 * dd4hep::cm, 50 * dd4hep::cm};     //{this, "localDistXY", {}};
        u_localDistXZ={};     //{this, "localDistXZ", {}};
        u_localDistYZ={};     //{this, "localDistYZ", {}};
        u_globalDistRPhi={};  //{this, "globalDistRPhi", {}};
        u_globalDistEtaPhi={};//{this, "globalDistEtaPhi", {}};
        u_dimScaledLocalDistXY={50.0*dd4hep::mm, 50.0*dd4hep::mm};// from https://eicweb.phy.anl.gov/EIC/detectors/athena/-/blob/master/calibrations/ffi_zdc.json


        app->SetDefaultParameter("ZDC:ZDCEcalIslandProtoClusters:splitCluster",             m_splitCluster);
        app->SetDefaultParameter("ZDC:ZDCEcalIslandProtoClusters:minClusterHitEdep",  m_minClusterHitEdep);
        app->SetDefaultParameter("ZDC:ZDCEcalIslandProtoClusters:minClusterCenterEdep",     m_minClusterCenterEdep);
        //app->SetDefaultParameter("ZDC:ZDCEcalIslandProtoClusters:inputHitCollection", m_inputHitCollection);
        //app->SetDefaultParameter("ZDC:ZDCEcalIslandProtoClusters:outputProtoClusterCollection",    m_outputProtoCollection);
        app->SetDefaultParameter("ZDC:ZDCEcalIslandProtoClusters:sectorDist",   m_sectorDist);
        app->SetDefaultParameter("ZDC:ZDCEcalIslandProtoClusters:localDistXY",   u_localDistXY);
        app->SetDefaultParameter("ZDC:ZDCEcalIslandProtoClusters:localDistXZ",   u_localDistXZ);
        app->SetDefaultParameter("ZDC:ZDCEcalIslandProtoClusters:localDistYZ",  u_localDistYZ);
        app->SetDefaultParameter("ZDC:ZDCEcalIslandProtoClusters:globalDistRPhi",    u_globalDistRPhi);
        app->SetDefaultParameter("ZDC:ZDCEcalIslandProtoClusters:globalDistEtaPhi",    u_globalDistEtaPhi);
        app->SetDefaultParameter("ZDC:ZDCEcalIslandProtoClusters:dimScaledLocalDistXY",    u_dimScaledLocalDistXY);
        app->SetDefaultParameter("ZDC:ZDCIslandProtoClusters:adjacencyMatrix", u_adjacencyMatrix);
        app->SetDefaultParameter("ZDC:ZDCIslandProtoClusters:geoServiceName", m_geoSvcName);
        app->SetDefaultParameter("ZDC:ZDCIslandProtoClusters:readoutClass", m_readout);
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
