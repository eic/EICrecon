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



class Cluster_factory_EcalEndcapPInsertClusters : public JMultifactory, CalorimeterClusterRecoCoG {

public:
    //------------------------------------------
    // Constructor
    Cluster_factory_EcalEndcapPInsertClusters(){
        DeclarePodioOutput<edm4eic::Cluster>("EcalEndcapPInsertClusters");
        DeclarePodioOutput<edm4eic::MCRecoClusterParticleAssociation>("EcalEndcapPInsertClusterAssociations");
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = japp; // GetApplication(); // TODO: NWB: FIXME
        m_log = japp->GetService<Log_service>()->logger("EcalEndcapPInsertClusters");

        //-------- Configuration Parameters ------------
        m_input_simhit_tag="EcalEndcapPInsertHits";
        m_input_protoclust_tag="EcalEndcapPInsertIslandProtoClusters";

        m_sampFrac=1.0;//{this, "samplingFraction", 1.0};
        m_logWeightBase=3.6;//{this, "logWeightBase", 3.6};
        m_depthCorrection=0.0;//{this, "depthCorrection", 0.0};
        m_energyWeight="log";//{this, "energyWeight", "log"};
        m_moduleDimZName="";//{this, "moduleDimZName", ""};
        // Constrain the cluster position eta to be within
        // the eta of the contributing hits. This is useful to avoid edge effects
        // for endcaps.
        m_enableEtaBounds=false;//{this, "enableEtaBounds", false};


        app->SetDefaultParameter("FEMC:EcalEndcapPInsertClusters:input_protoclust_tag",    m_input_protoclust_tag, "Name of input collection to use");
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertClusters:samplingFraction",             m_sampFrac);
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertClusters:logWeightBase",  m_logWeightBase);
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertClusters:depthCorrection",     m_depthCorrection);
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertClusters:energyWeight",   m_energyWeight);
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertClusters:moduleDimZName",   m_moduleDimZName);
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertClusters:enableEtaBounds",   m_enableEtaBounds);

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
        SetData("EcalEndcapPInsertClusters", m_outputClusters);
        SetData("EcalEndcapPInsertClusterAssociations", m_outputAssociations);

        m_outputClusters.clear(); // not really needed, but better to not leave dangling pointers around
        m_outputAssociations.clear();
    }
};
