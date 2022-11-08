// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef _ProtoCLuster_factory_EcalBarrelIslandProtoClusters_h_
#define _ProtoCLuster_factory_EcalBarrelIslandProtoClusters_h_

#include <random>

#include <JANA/JFactoryT.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/calorimetry/CalorimeterIslandCluster.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>

class ProtoCluster_factory_EcalBarrelSciGlassProtoClusters : public JFactoryT<edm4eic::ProtoCluster>, CalorimeterIslandCluster {

public:
    //------------------------------------------
    // Constructor
    ProtoCluster_factory_EcalBarrelSciGlassProtoClusters(){
        SetTag("EcalBarrelSciGlassProtoClusters");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();
        m_input_tag = "EcalBarrelSciGlassRecHits";

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

        app->SetDefaultParameter("BEMC:EcalBarrelSciGlassProtoClusters:input_tag", m_input_tag, "Name of input collection to use");
        app->SetDefaultParameter("BEMC:EcalBarrelSciGlassProtoClusters:splitCluster",             m_splitCluster);
        app->SetDefaultParameter("BEMC:EcalBarrelSciGlassProtoClusters:minClusterHitEdep",  m_minClusterHitEdep);
        app->SetDefaultParameter("BEMC:EcalBarrelSciGlassProtoClusters:minClusterCenterEdep",     m_minClusterCenterEdep);
        app->SetDefaultParameter("BEMC:EcalBarrelSciGlassProtoClusters:sectorDist",   m_sectorDist);
        app->SetDefaultParameter("BEMC:EcalBarrelSciGlassProtoClusters:localDistXY",   u_localDistXY);
        app->SetDefaultParameter("BEMC:EcalBarrelSciGlassProtoClusters:localDistXZ",   u_localDistXZ);
        app->SetDefaultParameter("BEMC:EcalBarrelSciGlassProtoClusters:localDistYZ",  u_localDistYZ);
        app->SetDefaultParameter("BEMC:EcalBarrelSciGlassProtoClusters:globalDistRPhi",    u_globalDistRPhi);
        app->SetDefaultParameter("BEMC:EcalBarrelSciGlassProtoClusters:globalDistEtaPhi",    u_globalDistEtaPhi);
        app->SetDefaultParameter("BEMC:EcalBarrelSciGlassProtoClusters:dimScaledLocalDistXY",    u_dimScaledLocalDistXY);
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

#endif // _ProtoCLuster_factory_EcalBarrelIslandProtoClusters_h_
