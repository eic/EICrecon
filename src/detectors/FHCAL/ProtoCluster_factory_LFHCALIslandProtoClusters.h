// Copyright 2023, Friederike Bock
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef _ProtoCluster_factory_LFHCALIslandProtoClusters_h_
#define _ProtoCluster_factory_LFHCALIslandProtoClusters_h_

#include <random>

#include <services/io/podio/JFactoryPodioT.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/calorimetry/CalorimeterIslandCluster.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>

class ProtoCluster_factory_LFHCALIslandProtoClusters : public eicrecon::JFactoryPodioT<edm4eic::ProtoCluster>, CalorimeterIslandCluster {

public:
    //------------------------------------------
    // Constructor
    ProtoCluster_factory_LFHCALIslandProtoClusters(){
        SetTag("LFHCALIslandProtoClusters");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();
        m_input_tag = "LFHCALRecHits";

        m_splitCluster=false;
        m_minClusterHitEdep=1 * dd4hep::MeV;
        m_minClusterCenterEdep=100.0 * dd4hep::MeV;

        // neighbour checking distances
        m_sectorDist=0 * dd4hep::cm;
        u_localDistXY={};     //{this, "localDistXY", {}};
        u_localDistXZ={};     //{this, "localDistXZ", {}};
        u_localDistYZ={};     //{this, "localDistYZ", {}};
        u_globalDistRPhi={};  //{this, "globalDistRPhi", {}};
        u_globalDistEtaPhi={};//{this, "globalDistEtaPhi", {}};
        u_dimScaledLocalDistXY={};

        // adjacency matrix
        m_geoSvcName = "geoServiceName";
        // Magic constants:
        //  24 - number of sectors
        //  5  - number of towers per sector
        //  moduleIDx
        //  moduleIDy
        //  towerx
        //  towery
        //  rlayerz

        std::string cellIdx_1  = "(54*2-moduleIDx_1*2+towerx_1)";
        std::string cellIdx_2  = "(54*2-moduleIDx_2*2+towerx_2)";
        std::string cellIdy_1  = "(54*2-moduleIDy_1*2+towery_1)";
        std::string cellIdy_2  = "(54*2-moduleIDy_2*2+towery_2)";
        std::string cellIdz_1  = "rlayerz_1";
        std::string cellIdz_2  = "rlayerz_2";
        std::string deltaX     = Form("abs(%s-%s)", cellIdx_2.data(), cellIdx_1.data());
        std::string deltaY     = Form("abs(%s-%s)", cellIdy_2.data(), cellIdy_1.data());
        std::string deltaZ     = Form("abs(%s-%s)", cellIdz_2.data(), cellIdz_1.data());
        std::string neighbor   = Form("(%s+%s+%s==1)", deltaX.data(), deltaY.data(), deltaZ.data());
        std::string corner2D   = Form("((%s==0&&%s==1&&%s==1)||(%s==1&&%s==0&&%s==1)||(%s==1&&%s==1&&%s==0))",
                                  deltaZ.data(), deltaX.data(), deltaY.data(),
                                  deltaZ.data(), deltaX.data(), deltaY.data(),
                                  deltaZ.data(), deltaX.data(), deltaY.data());
        u_adjacencyMatrix = Form("%s||%s", neighbor.data(), corner2D.data());
//         u_adjacencyMatrix = Form("%s==1", neighbor.data());
//         u_adjacencyMatrix = Form("%s==1", corner2D.data());
        std::remove(u_adjacencyMatrix.begin(), u_adjacencyMatrix.end(), ' ');
        m_readout = "LFHCALHits";

        app->SetDefaultParameter("FHCAL:LFHCALIslandProtoClusters:splitCluster",          m_splitCluster);
        app->SetDefaultParameter("FHCAL:LFHCALIslandProtoClusters:minClusterHitEdep",     m_minClusterHitEdep);
        app->SetDefaultParameter("FHCAL:LFHCALIslandProtoClusters:minClusterCenterEdep",  m_minClusterCenterEdep);
        app->SetDefaultParameter("FHCAL:LFHCALIslandProtoClusters:sectorDist",            m_sectorDist);
        app->SetDefaultParameter("FHCAL:LFHCALIslandProtoClusters:localDistXY",           u_localDistXY);
        app->SetDefaultParameter("FHCAL:LFHCALIslandProtoClusters:localDistXZ",           u_localDistXZ);
        app->SetDefaultParameter("FHCAL:LFHCALIslandProtoClusters:localDistYZ",           u_localDistYZ);
        app->SetDefaultParameter("FHCAL:LFHCALIslandProtoClusters:globalDistRPhi",        u_globalDistRPhi);
        app->SetDefaultParameter("FHCAL:LFHCALIslandProtoClusters:globalDistEtaPhi",      u_globalDistEtaPhi);
        app->SetDefaultParameter("FHCAL:LFHCALIslandProtoClusters:dimScaledLocalDistXY",  u_dimScaledLocalDistXY);
        app->SetDefaultParameter("FHCAL:LFHCALIslandProtoClusters:adjacencyMatrix",       u_adjacencyMatrix);
        app->SetDefaultParameter("FHCAL:LFHCALIslandProtoClusters:geoServiceName",        m_geoSvcName);
        app->SetDefaultParameter("FHCAL:LFHCALIslandProtoClusters:readoutClass",          m_readout);


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

#endif // _ProtoCluster_factory_LFHCALIslandProtoClusters_h_
