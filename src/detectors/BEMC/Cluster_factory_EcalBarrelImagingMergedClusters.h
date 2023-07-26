// Copyright 2022, Thomas Britton
// Subject to the terms in the LICENSE file found in the top-level directory.
//
#pragma once

#include <random>

#include <extensions/jana/JChainFactoryT.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/calorimetry/TruthEnergyPositionClusterMerger.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>


class Cluster_factory_EcalBarrelImagingMergedClusters : public JChainFactoryT<edm4eic::Cluster>, TruthEnergyPositionClusterMerger {

public:
    //------------------------------------------
    // Constructor
    Cluster_factory_EcalBarrelImagingMergedClusters(std::vector<std::string> default_input_tags)
    : JChainFactoryT<edm4eic::Cluster>(std::move(default_input_tags)) {
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        InitDataTags(GetPluginName() + ":" + GetTag());

        initialize();
    }


    //------------------------------------------
    // Process
    void Process(const std::shared_ptr<const JEvent> &event) override{

        // Prefill inputs
        m_inputMCParticles     = event->Get<edm4hep::MCParticle>(GetInputTags()[0]);
        m_energyClusters       = event->Get<edm4eic::Cluster>(GetInputTags()[1]);
        m_energyAssociations   = event->Get<edm4eic::MCRecoClusterParticleAssociation>(GetInputTags()[2]);
        m_positionClusters     = event->Get<edm4eic::Cluster>(GetInputTags()[3]);
        m_positionAssociations = event->Get<edm4eic::MCRecoClusterParticleAssociation>(GetInputTags()[4]);

        // Call Process for generic algorithm
        execute();

        // Hand owner of algorithm objects over to JANA
        Set(m_outputClusters);
        event->Insert(m_outputAssociations, "EcalBarrelImagingMergedClustersAssoc");
        m_outputClusters.clear(); // not really needed, but better to not leave dangling pointers around
        m_outputAssociations.clear();
    }
};
