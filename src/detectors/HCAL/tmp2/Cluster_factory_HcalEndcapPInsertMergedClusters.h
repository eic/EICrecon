// Copyright 2022, Thomas Britton
// Subject to the terms in the LICENSE file found in the top-level directory.
//
#pragma once

#include <random>

#include <JANA/JFactoryT.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/calorimetry/CalorimeterClusterMerger.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>



class Cluster_factory_HcalEndcapPInsertMergedClusters : public JFactoryT<edm4eic::Cluster>, CalorimeterClusterMerger {

public:
    //------------------------------------------
    // Constructor
    Cluster_factory_HcalEndcapPInsertMergedClusters(){
        SetTag("HcalEndcapPInsertMergedClusters");
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();
        //-------- Configuration Parameters ------------
        m_input_tag="HcalEndcapPInsertClusters";
        m_inputAssociations_tag="HcalEndcapPInsertClusterAssociations";

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
        m_inputClusters=event->Get<edm4eic::Cluster>(m_input_tag);
        m_inputAssociations=event->Get<edm4eic::MCRecoClusterParticleAssociation>(m_inputAssociations_tag); 

        // Call Process for generic algorithm
        AlgorithmProcess();

        //outputs
        // Hand owner of algorithm objects over to JANA
        Set(m_outputClusters);
        event->Insert(m_outputAssociations, "HcalEndcapPInsertMergedClusterAssociations");
        m_outputClusters.clear(); // not really needed, but better to not leave dangling pointers around
        m_outputAssociations.clear();
    }

private:
    // Name of input data type (collection)
    std::string              m_input_tag;
    std::string              m_inputAssociations_tag;
};

