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

class ProtoCluster_factory_EcalLumiSpecIslandProtoClusters : public eicrecon::JFactoryPodioT<edm4eic::ProtoCluster>, CalorimeterIslandCluster {

public:
    //------------------------------------------
    // Constructor
    ProtoCluster_factory_EcalLumiSpecIslandProtoClusters(){
        SetTag("EcalLumiSpecIslandProtoClusters");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();
        m_input_tag = "EcalLumiSpecRecHits";

        // adjacency matrix
        m_geoSvcName = "GeoSvc";
        u_adjacencyMatrix = "(sector_1 == sector_2) && ((abs(floor(module_1 / 10) - floor(module_2 / 10)) + abs(fmod(module_1, 10) - fmod(module_2, 10))) == 1)";
        u_adjacencyMatrix.erase(
          std::remove_if(u_adjacencyMatrix.begin(), u_adjacencyMatrix.end(), ::isspace),
          u_adjacencyMatrix.end());
        m_readout = "LumiSpecCALHits";

        // neighbour checking distances
        m_sectorDist=0.0 * dd4hep::cm;             // from ATHENA reconstruction.py
        u_localDistXY={};     //{this, "localDistXY", {}};
        u_localDistXZ={};     //{this, "localDistXZ", {}};
        u_localDistYZ={};     //{this, "localDistYZ", {}};
        u_globalDistRPhi={};  //{this, "globalDistRPhi", {}};
        u_globalDistEtaPhi={};//{this, "globalDistEtaPhi", {}};
        u_dimScaledLocalDistXY={};

        m_splitCluster=true;
        m_minClusterHitEdep=1.0 * dd4hep::MeV;
        m_minClusterCenterEdep=30.0 * dd4hep::MeV;
        u_transverseEnergyProfileMetric = "localDistXY";
        u_transverseEnergyProfileScale = 10. * dd4hep::mm;

        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecIslandProtoClusters:input_tag",        m_input_tag, "Name of input collection to use");
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecIslandProtoClusters:geoServiceName", m_geoSvcName);
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecIslandProtoClusters:readoutClass", m_readout);
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecIslandProtoClusters:sectorDist",   m_sectorDist);
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecIslandProtoClusters:localDistXY",   u_localDistXY);
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecIslandProtoClusters:localDistXZ",   u_localDistXZ);
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecIslandProtoClusters:localDistYZ",  u_localDistYZ);
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecIslandProtoClusters:globalDistRPhi",    u_globalDistRPhi);
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecIslandProtoClusters:globalDistEtaPhi",    u_globalDistEtaPhi);
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecIslandProtoClusters:dimScaledLocalDistXY",    u_dimScaledLocalDistXY);
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecIslandProtoClusters:adjacencyMatrix", u_adjacencyMatrix);
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecIslandProtoClusters:splitCluster", m_splitCluster);
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecIslandProtoClusters:minClusterHitEdep", m_minClusterHitEdep);
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecIslandProtoClusters:minClusterCenterEdep", m_minClusterCenterEdep);
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecIslandProtoClusters:transverseEnergyProfileMetric", u_transverseEnergyProfileMetric);
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecIslandProtoClusters:transverseEnergyProfileScale", u_transverseEnergyProfileScale);
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
