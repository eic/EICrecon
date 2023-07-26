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

class ProtoCluster_factory_HcalEndcapPInsertIslandProtoClusters : public JChainFactoryT<edm4eic::ProtoCluster>, CalorimeterIslandCluster {

public:
    //------------------------------------------
    // Constructor
    ProtoCluster_factory_HcalEndcapPInsertIslandProtoClusters(std::vector<std::string> default_input_tags)
    : JChainFactoryT<edm4eic::ProtoCluster>(std::move(default_input_tags)) {
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        InitDataTags(GetPluginName() + ":" + GetTag());

        auto app = GetApplication();

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

        m_splitCluster=true;
        m_minClusterHitEdep=0.0 * dd4hep::MeV;
        m_minClusterCenterEdep=30.0 * dd4hep::MeV;
        u_transverseEnergyProfileMetric = "globalDistEtaPhi";
        u_transverseEnergyProfileScale = 1.;

        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertIslandProtoClusters:geoServiceName", m_geoSvcName);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertIslandProtoClusters:readoutClass", m_readout);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertIslandProtoClusters:sectorDist",   m_sectorDist);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertIslandProtoClusters:localDistXY",   u_localDistXY);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertIslandProtoClusters:localDistXZ",   u_localDistXZ);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertIslandProtoClusters:localDistYZ",  u_localDistYZ);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertIslandProtoClusters:globalDistRPhi",    u_globalDistRPhi);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertIslandProtoClusters:globalDistEtaPhi",    u_globalDistEtaPhi);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertIslandProtoClusters:dimScaledLocalDistXY",    u_dimScaledLocalDistXY);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertIslandProtoClusters:adjacencyMatrix", u_adjacencyMatrix);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertIslandProtoClusters:splitCluster", m_splitCluster);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertIslandProtoClusters:minClusterHitEdep", m_minClusterHitEdep);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertIslandProtoClusters:minClusterCenterEdep", m_minClusterCenterEdep);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertIslandProtoClusters:transverseEnergyProfileMetric", u_transverseEnergyProfileMetric);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertIslandProtoClusters:transverseEnergyProfileScale", u_transverseEnergyProfileScale);
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
        hits = event->Get<edm4eic::CalorimeterHit>(GetInputTags()[0]);

        // Call Process for generic algorithm
        AlgorithmProcess();

        // Hand owner of algorithm objects over to JANA
        Set(protoClusters);
        protoClusters.clear(); // not really needed, but better to not leave dangling pointers around
    }
};
