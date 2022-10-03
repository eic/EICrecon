// Copyright 2022, Thomas Britton
// Subject to the terms in the LICENSE file found in the top-level directory.
//
#pragma once

#include <random>

#include <JANA/JFactoryT.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/calorimetry/CalorimeterClusterRecoCoG.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>



class Cluster_factory_HcalEndcapNTruthClusters : public JFactoryT<edm4eic::Cluster>, CalorimeterClusterRecoCoG {

public:
    //------------------------------------------
    // Constructor
    Cluster_factory_HcalEndcapNTruthClusters(){
        SetTag("HcalEndcapNTruthClusters");
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();
        //-------- Configuration Parameters ------------
        m_input_simhit_tag="HcalEndcapNHits";
        m_input_protoclust_tag="HcalEndcapNTruthProtoClusters";

        m_sampFrac=1.0;//{this, "samplingFraction", 1.0};
        m_logWeightBase=3.6;//{this, "logWeightBase", 3.6};
        m_depthCorrection=0.0;//{this, "depthCorrection", 0.0};
        m_energyWeight="log";//{this, "energyWeight", "log"};
        m_moduleDimZName="";//{this, "moduleDimZName", ""};
        // Constrain the cluster position eta to be within
        // the eta of the contributing hits. This is useful to avoid edge effects
        // for endcaps.
        m_enableEtaBounds=false;//{this, "enableEtaBounds", false};


        app->SetDefaultParameter("ZDC:HcalEndcapNTruthClusters:samplingFraction",             m_sampFrac);
        app->SetDefaultParameter("ZDC:HcalEndcapNTruthClusters:logWeightBase",  m_logWeightBase);
        app->SetDefaultParameter("ZDC:HcalEndcapNTruthClusters:depthCorrection",     m_depthCorrection);
        app->SetDefaultParameter("ZDC:HcalEndcapNTruthClusters:input_simhit_tag", m_input_simhit_tag);
        app->SetDefaultParameter("ZDC:HcalEndcapNTruthClusters:input_protoclust_tag", m_input_protoclust_tag);
        app->SetDefaultParameter("ZDC:HcalEndcapNTruthClusters:energyWeight",   m_energyWeight);
        app->SetDefaultParameter("ZDC:HcalEndcapNTruthClusters:moduleDimZName",   m_moduleDimZName);
        app->SetDefaultParameter("ZDC:HcalEndcapNTruthClusters:enableEtaBounds",   m_enableEtaBounds);

        m_geoSvc = app->template GetService<JDD4hep_service>();

        std::string tag=this->GetTag();
        std::shared_ptr<spdlog::logger> m_log = app->GetService<Log_service>()->logger(tag);

        // Get log level from user parameter or default
        std::string log_level_str = "info";
        auto pm = app->GetJParameterManager();
        pm->SetDefaultParameter(tag + ":LogLevel", log_level_str, "verbosity: trace, debug, info, warn, err, critical, off");
        m_log->set_level(eicrecon::ParseLogLevel(log_level_str));


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
        event->Insert(m_outputAssociations, "HcalEndcapNTruthClusterAssociations");
        m_outputClusters.clear(); // not really needed, but better to not leave dangling pointers around
        m_outputAssociations.clear();
    }
};

