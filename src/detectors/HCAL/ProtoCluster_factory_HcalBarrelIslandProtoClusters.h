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

class ProtoCluster_factory_HcalBarrelIslandProtoClusters : public JFactoryT<edm4eic::ProtoCluster>, CalorimeterIslandCluster {

public:
    //------------------------------------------
    // Constructor
    ProtoCluster_factory_HcalBarrelIslandProtoClusters(){
        SetTag("HcalBarrelIslandProtoClusters");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();
        m_input_tag = "HcalBarrelRecHits";

        m_splitCluster=false;              // from https://eicweb.phy.anl.gov/EIC/detectors/athena/-/blob/master/calibrations/ffi_zdc.json
        m_minClusterHitEdep=3.0 * dd4hep::MeV;    // from https://eicweb.phy.anl.gov/EIC/detectors/athena/-/blob/master/calibrations/ffi_zdc.json
        m_minClusterCenterEdep=30.0 * dd4hep::MeV; // from https://eicweb.phy.anl.gov/EIC/detectors/athena/-/blob/master/calibrations/ffi_zdc.json

        // adjacency matrix
        m_geoSvcName = "GeoSvc";
        // Magic constants:
        //   32 - number of sectors
        //   24 - number of towers per sector per phi
        u_adjacencyMatrix =
          "("
          "  abs(fmod(tower_1, 24) - fmod(tower_2, 24))"
          "  + min("
          "      abs((sector_1 - sector_2) * 2 + floor(tower_1 / 24) - floor(tower_2 / 24)),"
          "      32 * 2 - abs((sector_1 - sector_2) * 2 + floor(tower_1 / 24) - floor(tower_2 / 24))"
          "    )"
          ") == 1";
        u_adjacencyMatrix.erase(
          std::remove_if(u_adjacencyMatrix.begin(), u_adjacencyMatrix.end(), ::isspace),
          u_adjacencyMatrix.end());
        m_readout = "HcalBarrelHits";

        // neighbour checking distances
        m_sectorDist=5.0 * dd4hep::cm;             // from ATHENA reconstruction.py
        u_localDistXY={15*dd4hep::mm, 15*dd4hep::mm};     //{this, "localDistXY", {}};
        u_localDistXZ={};     //{this, "localDistXZ", {}};
        u_localDistYZ={};     //{this, "localDistYZ", {}};
        u_globalDistRPhi={};  //{this, "globalDistRPhi", {}};
        u_globalDistEtaPhi={};//{this, "globalDistEtaPhi", {}};
        u_dimScaledLocalDistXY={50.0*dd4hep::mm, 50.0*dd4hep::mm};// from https://eicweb.phy.anl.gov/EIC/detectors/athena/-/blob/master/calibrations/ffi_zdc.json


        app->SetDefaultParameter("HCAL:HcalBarrelIslandProtoClusters:splitCluster",             m_splitCluster);
        app->SetDefaultParameter("HCAL:HcalBarrelIslandProtoClusters:minClusterHitEdep",  m_minClusterHitEdep);
        app->SetDefaultParameter("HCAL:HcalBarrelIslandProtoClusters:minClusterCenterEdep",     m_minClusterCenterEdep);
        app->SetDefaultParameter("HCAL:HcalBarrelIslandProtoClusters:sectorDist",   m_sectorDist);
        app->SetDefaultParameter("HCAL:HcalBarrelIslandProtoClusters:localDistXY",   u_localDistXY);
        app->SetDefaultParameter("HCAL:HcalBarrelIslandProtoClusters:localDistXZ",   u_localDistXZ);
        app->SetDefaultParameter("HCAL:HcalBarrelIslandProtoClusters:localDistYZ",  u_localDistYZ);
        app->SetDefaultParameter("HCAL:HcalBarrelIslandProtoClusters:globalDistRPhi",    u_globalDistRPhi);
        app->SetDefaultParameter("HCAL:HcalBarrelIslandProtoClusters:globalDistEtaPhi",    u_globalDistEtaPhi);
        app->SetDefaultParameter("HCAL:HcalBarrelIslandProtoClusters:dimScaledLocalDistXY",    u_dimScaledLocalDistXY);
        app->SetDefaultParameter("HCAL:HcalBarrelIslandProtoClusters:adjacencyMatrix", u_adjacencyMatrix);
        app->SetDefaultParameter("HCAL:HcalBarrelIslandProtoClusters:geoServiceName", m_geoSvcName);
        app->SetDefaultParameter("HCAL:HcalBarrelIslandProtoClusters:readoutClass", m_readout);
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
