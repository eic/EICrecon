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

class ProtoCluster_factory_HcalEndcapNIslandProtoClusters : public eicrecon::JFactoryPodioT<edm4eic::ProtoCluster>, CalorimeterIslandCluster {

public:
    //------------------------------------------
    // Constructor
    ProtoCluster_factory_HcalEndcapNIslandProtoClusters(){
        SetTag("HcalEndcapNIslandProtoClusters");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();
        m_input_tag = "HcalEndcapNMergedHits";

        m_splitCluster=true;              // https://eicweb.phy.anl.gov/EIC/juggler/-/blob/main/JugReco/src/components/CalorimeterIslandCluster.cpp
        m_minClusterHitEdep=0.0 * dd4hep::MeV;    // https://eicweb.phy.anl.gov/EIC/juggler/-/blob/main/JugReco/src/components/CalorimeterIslandCluster.cpp
        m_minClusterCenterEdep=30.0 * dd4hep::MeV; // from ATHENA's reconstruction.py

        // adjacency matrix
        m_geoSvcName = "GeoSvc";
        u_adjacencyMatrix = "";
        u_adjacencyMatrix.erase(
          std::remove_if(u_adjacencyMatrix.begin(), u_adjacencyMatrix.end(), ::isspace),
          u_adjacencyMatrix.end());
        m_readout = "";

        // neighbour checking distances
        m_sectorDist=5.0 * dd4hep::cm;             // https://eicweb.phy.anl.gov/EIC/juggler/-/blob/main/JugReco/src/components/CalorimeterIslandCluster.cpp
        u_localDistXY={15*dd4hep::mm, 15*dd4hep::mm};     //{this, "localDistXY", {}};
        u_localDistXZ={};     //{this, "localDistXZ", {}};
        u_localDistYZ={};     //{this, "localDistYZ", {}};
        u_globalDistRPhi={};  //{this, "globalDistRPhi", {}};
        u_globalDistEtaPhi={};//{this, "globalDistEtaPhi", {}};
        u_dimScaledLocalDistXY={15.0*dd4hep::mm, 15.0*dd4hep::mm};// from ATHENA's reconstruction.py


        app->SetDefaultParameter("EHCAL:HcalEndcapNIslandProtoClusters:splitCluster",             m_splitCluster);
        app->SetDefaultParameter("EHCAL:HcalEndcapNIslandProtoClusters:minClusterHitEdep",  m_minClusterHitEdep);
        app->SetDefaultParameter("EHCAL:HcalEndcapNIslandProtoClusters:minClusterCenterEdep",     m_minClusterCenterEdep);
        app->SetDefaultParameter("EHCAL:HcalEndcapNIslandProtoClusters:sectorDist",   m_sectorDist);
        app->SetDefaultParameter("EHCAL:HcalEndcapNIslandProtoClusters:localDistXY",   u_localDistXY);
        app->SetDefaultParameter("EHCAL:HcalEndcapNIslandProtoClusters:localDistXZ",   u_localDistXZ);
        app->SetDefaultParameter("EHCAL:HcalEndcapNIslandProtoClusters:localDistYZ",  u_localDistYZ);
        app->SetDefaultParameter("EHCAL:HcalEndcapNIslandProtoClusters:globalDistRPhi",    u_globalDistRPhi);
        app->SetDefaultParameter("EHCAL:HcalEndcapNIslandProtoClusters:globalDistEtaPhi",    u_globalDistEtaPhi);
        app->SetDefaultParameter("EHCAL:HcalEndcapNIslandProtoClusters:dimScaledLocalDistXY",    u_dimScaledLocalDistXY);
        app->SetDefaultParameter("EHCAL:HcalEndcapNIslandProtoClusters:adjacencyMatrix", u_adjacencyMatrix);
        app->SetDefaultParameter("EHCAL:HcalEndcapNIslandProtoClusters:geoServiceName", m_geoSvcName);
        app->SetDefaultParameter("EHCAL:HcalEndcapNIslandProtoClusters:readoutClass", m_readout);
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
