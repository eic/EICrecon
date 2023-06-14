// Copyright 2022, Thomas Britton
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <random>

#include <edm4eic/ClusterCollection.h>

#include <services/io/podio/JFactoryPodioT.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/calorimetry/CalorimeterClusterRecoCoG.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>



// Dummy factory for JFactoryGeneratorT
class Association_factory_EcalLumiSpecTruthClusterAssociations : public eicrecon::JFactoryPodioT<edm4eic::MCRecoClusterParticleAssociation> {

public:
    //------------------------------------------
    // Constructor
    Association_factory_EcalLumiSpecTruthClusterAssociations(){
        SetTag("EcalLumiSpecTruthClusterAssociations");
    }
};



class Cluster_factory_EcalLumiSpecTruthClusters : public eicrecon::JFactoryPodioT<edm4eic::Cluster>, CalorimeterClusterRecoCoG {

public:
    //------------------------------------------
    // Constructor
    Cluster_factory_EcalLumiSpecTruthClusters(){
        SetTag("EcalLumiSpecTruthClusters");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();
        //-------- Configuration Parameters ------------
        m_input_simhit_tag="LumiSpecCALHits";
        m_input_protoclust_tag="EcalLumiSpecTruthProtoClusters";

        m_sampFrac=1.0;//{this, "samplingFraction", 1.0};
        m_logWeightBase=4.6;//{this, "logWeightBase", 3.6};
        m_depthCorrection=0.0;//{this, "depthCorrection", 0.0};
        m_energyWeight="log";//{this, "energyWeight", "log"};
        m_moduleDimZName="";//{this, "moduleDimZName", ""};
        // Constrain the cluster position eta to be within
        // the eta of the contributing hits. This is useful to avoid edge effects
        // for endcaps.
        m_enableEtaBounds=false;//{this, "enableEtaBounds", false};


        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecTruthClusters:input_protoclust_tag",    m_input_protoclust_tag, "Name of input collection to use");
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecTruthClusters:samplingFraction",             m_sampFrac);
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecTruthClusters:logWeightBase",  m_logWeightBase);
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecTruthClusters:depthCorrection",     m_depthCorrection);
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecTruthClusters:energyWeight",   m_energyWeight);
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecTruthClusters:moduleDimZName",   m_moduleDimZName);
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecTruthClusters:enableEtaBounds",   m_enableEtaBounds);

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
        m_inputSimhits=event->Get<edm4hep::SimCalorimeterHit>(m_input_simhit_tag);
        m_inputProto=event->Get<edm4eic::ProtoCluster>(m_input_protoclust_tag);

        // Call Process for generic algorithm
        AlgorithmProcess();


        //outputs

        // Hand owner of algorithm objects over to JANA
        Set(m_outputClusters);
        event->Insert(m_outputAssociations, "EcalLumiSpecTruthClusterAssociations");
        m_outputClusters.clear(); // not really needed, but better to not leave dangling pointers around
        m_outputAssociations.clear();
    }
};
