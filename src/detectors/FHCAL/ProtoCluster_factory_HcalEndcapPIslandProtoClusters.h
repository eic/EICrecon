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

class ProtoCluster_factory_HcalEndcapPIslandProtoClusters : public eicrecon::JFactoryPodioT<edm4eic::ProtoCluster>, CalorimeterIslandCluster {

public:
    //------------------------------------------
    // Constructor
    ProtoCluster_factory_HcalEndcapPIslandProtoClusters(){
        SetTag("HcalEndcapPIslandProtoClusters");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();
        m_input_tag = "HcalEndcapPMergedHits";

        // adjacency matrix
        m_geoSvcName = "GeoSvc";
        u_adjacencyMatrix = "";
        u_adjacencyMatrix.erase(
          std::remove_if(u_adjacencyMatrix.begin(), u_adjacencyMatrix.end(), ::isspace),
          u_adjacencyMatrix.end());
        m_readout = "";

        // neighbour checking distances
        m_sectorDist=5.0 * dd4hep::cm;             // https://eicweb.phy.anl.gov/EIC/juggler/-/blob/main/JugReco/src/components/CalorimeterIslandCluster.cpp
        u_localDistXY={15.0*dd4hep::mm, 15.0*dd4hep::mm};     //{this, "localDistXY", {}};
        u_localDistXZ={};     //{this, "localDistXZ", {}};
        u_localDistYZ={};     //{this, "localDistYZ", {}};
        u_globalDistRPhi={};  //{this, "globalDistRPhi", {}};
        u_globalDistEtaPhi={};//{this, "globalDistEtaPhi", {}};
        u_dimScaledLocalDistXY={15.0*dd4hep::mm, 15.0*dd4hep::mm};// from ATHENA's reconstruction.py

        m_splitCluster=true;
        m_minClusterHitEdep=0.0 * dd4hep::MeV;
        m_minClusterCenterEdep=30.0 * dd4hep::MeV;
        u_transverseEnergyProfileMetric = "globalDistEtaPhi";
        u_transverseEnergyProfileScale = 1.;

        app->SetDefaultParameter("FHCAL:HcalEndcapPIslandProtoClusters:geoServiceName", m_geoSvcName);
        app->SetDefaultParameter("FHCAL:HcalEndcapPIslandProtoClusters:readoutClass", m_readout);
        app->SetDefaultParameter("FHCAL:HcalEndcapPIslandProtoClusters:sectorDist",   m_sectorDist);
        app->SetDefaultParameter("FHCAL:HcalEndcapPIslandProtoClusters:localDistXY",   u_localDistXY);
        app->SetDefaultParameter("FHCAL:HcalEndcapPIslandProtoClusters:localDistXZ",   u_localDistXZ);
        app->SetDefaultParameter("FHCAL:HcalEndcapPIslandProtoClusters:localDistYZ",  u_localDistYZ);
        app->SetDefaultParameter("FHCAL:HcalEndcapPIslandProtoClusters:globalDistRPhi",    u_globalDistRPhi);
        app->SetDefaultParameter("FHCAL:HcalEndcapPIslandProtoClusters:globalDistEtaPhi",    u_globalDistEtaPhi);
        app->SetDefaultParameter("FHCAL:HcalEndcapPIslandProtoClusters:dimScaledLocalDistXY",    u_dimScaledLocalDistXY);
        app->SetDefaultParameter("FHCAL:HcalEndcapPIslandProtoClusters:adjacencyMatrix", u_adjacencyMatrix);
        app->SetDefaultParameter("FHCAL:HcalEndcapPIslandProtoClusters:splitCluster", m_splitCluster);
        app->SetDefaultParameter("FHCAL:HcalEndcapPIslandProtoClusters:minClusterHitEdep", m_minClusterHitEdep);
        app->SetDefaultParameter("FHCAL:HcalEndcapPIslandProtoClusters:minClusterCenterEdep", m_minClusterCenterEdep);
        app->SetDefaultParameter("FHCAL:HcalEndcapPIslandProtoClusters:transverseEnergyProfileMetric", u_transverseEnergyProfileMetric);
        app->SetDefaultParameter("FHCAL:HcalEndcapPIslandProtoClusters:transverseEnergyProfileScale", u_transverseEnergyProfileScale);
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
        auto hits_coll = static_cast<const edm4eic::CalorimeterHitCollection*>(event->GetCollectionBase(m_input_tag));

        // Call Process for generic algorithm
        auto protoclust_coll = AlgorithmProcess(*hits_coll);

        // Hand algorithm objects over to JANA
        SetCollection(std::move(protoclust_coll));
    }
};
