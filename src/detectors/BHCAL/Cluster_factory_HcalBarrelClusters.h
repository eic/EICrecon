// Copyright 2022, Thomas Britton
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <random>

#include <JANA/JMultifactory.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/calorimetry/CalorimeterClusterRecoCoG.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>



class Cluster_factory_HcalBarrelClusters : public JMultifactory, CalorimeterClusterRecoCoG {

public:
    //------------------------------------------
    // Constructor
    Cluster_factory_HcalBarrelClusters(){
        DeclarePodioOutput<edm4eic::Cluster>("HcalBarrelClusters");
        DeclarePodioOutput<edm4eic::MCRecoClusterParticleAssociation>("HcalBarrelClusterAssociations");
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = japp; // GetApplication(); // TODO: NWB: FIXME after JANA2 v2.1.1
        m_log = app->GetService<Log_service>()->logger("HcalBarrelClusters");

        //-------- Configuration Parameters ------------
        m_input_simhit_tag="HcalBarrelHits";
        m_input_protoclust_tag="HcalBarrelIslandProtoClusters";

        m_sampFrac=1.0;//{this, "samplingFraction", 1.0};
        m_logWeightBase=6.2;//{this, "logWeightBase", 3.6};
        m_depthCorrection=0.0;//{this, "depthCorrection", 0.0};
        m_energyWeight="log";//{this, "energyWeight", "log"};
        m_moduleDimZName="";//{this, "moduleDimZName", ""};
        // Constrain the cluster position eta to be within
        // the eta of the contributing hits. This is useful to avoid edge effects
        // for endcaps.
        m_enableEtaBounds=false;//{this, "enableEtaBounds", false};


        app->SetDefaultParameter("BHCAL:HcalBarrelClusters:samplingFraction",             m_sampFrac);
        app->SetDefaultParameter("BHCAL:HcalBarrelClusters:logWeightBase",  m_logWeightBase);
        app->SetDefaultParameter("BHCAL:HcalBarrelClusters:depthCorrection",     m_depthCorrection);
        app->SetDefaultParameter("BHCAL:HcalBarrelClusters:input_simhit_tag", m_input_simhit_tag);
        app->SetDefaultParameter("BHCAL:HcalBarrelClusters:input_protoclust_tag", m_input_protoclust_tag);
        app->SetDefaultParameter("BHCAL:HcalBarrelClusters:energyWeight",   m_energyWeight);
        app->SetDefaultParameter("BHCAL:HcalBarrelClusters:moduleDimZName",   m_moduleDimZName);
        app->SetDefaultParameter("BHCAL:HcalBarrelClusters:enableEtaBounds",   m_enableEtaBounds);

        m_geoSvc = app->template GetService<JDD4hep_service>();

        AlgorithmInit(m_log);
    }

    //------------------------------------------
    // ChangeRun
    void BeginRun(const std::shared_ptr<const JEvent> &event) override{
        AlgorithmChangeRun();
    }

    //------------------------------------------
    // Process
    void Process(const std::shared_ptr<const JEvent> &event) override{

        // Prefill inputs
        m_inputSimhits=event->Get<edm4hep::SimCalorimeterHit>(m_input_simhit_tag);
        m_inputProto=event->Get<edm4eic::ProtoCluster>(m_input_protoclust_tag);

        // Call Process for generic algorithm
        AlgorithmProcess();


        // Hand owner of algorithm objects over to JANA
        SetData("HcalBarrelClusters", m_outputClusters);
        SetData("HcalBarrelClusterAssociations", m_outputAssociations);

        m_outputClusters.clear(); // not really needed, but better to not leave dangling pointers around
        m_outputAssociations.clear();
    }
};
