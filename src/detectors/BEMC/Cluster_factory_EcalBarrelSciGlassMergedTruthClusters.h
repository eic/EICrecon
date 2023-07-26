// Copyright 2022, Thomas Britton
// Subject to the terms in the LICENSE file found in the top-level directory.
//
#pragma once

#include <random>

#include "extensions/jana/JChainFactoryT.h"
#include "services/geometry/dd4hep/JDD4hep_service.h"
#include "algorithms/calorimetry/CalorimeterClusterMerger.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"



class Cluster_factory_EcalBarrelSciGlassMergedTruthClusters : public JChainFactoryT<edm4eic::Cluster>, CalorimeterClusterMerger {

public:
    //------------------------------------------
    // Constructor
    Cluster_factory_EcalBarrelSciGlassMergedTruthClusters(std::vector<std::string> default_input_tags)
    : JChainFactoryT<edm4eic::Cluster>(std::move(default_input_tags)) {
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        InitDataTags(GetPluginName() + ":" + GetTag());

        auto app = GetApplication();

        std::string tag=this->GetTag();
        std::shared_ptr<spdlog::logger> m_log = app->GetService<Log_service>()->logger(tag);

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
        m_inputClusters=event->Get<edm4eic::Cluster>(GetInputTags()[0]);
        m_inputAssociations=event->Get<edm4eic::MCRecoClusterParticleAssociation>(GetInputTags()[1]);

        // Call Process for generic algorithm
        AlgorithmProcess();

        //outputs
        // Hand owner of algorithm objects over to JANA
        Set(m_outputClusters);
        event->Insert(m_outputAssociations, "EcalBarrelMergedClusterAssociations");
        m_outputClusters.clear(); // not really needed, but better to not leave dangling pointers around
        m_outputAssociations.clear();
    }
};
