// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include "algorithms/calorimetry/TruthEnergyPositionClusterMerger.h"
#include "services/geometry/dd4hep/JDD4hep_service.h"
#include "extensions/jana/JChainMultifactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"


namespace eicrecon {

class TruthEnergyPositionClusterMerger_factoryT :
    public JChainMultifactoryT<NoConfig>,
    public SpdlogMixin {

  public:
    using SpdlogMixin::logger;

    explicit TruthEnergyPositionClusterMerger_factoryT(
        std::string tag,
        const std::vector<std::string>& input_tags,
        const std::vector<std::string>& output_tags)
    : JChainMultifactoryT<NoConfig>(std::move(tag), input_tags, output_tags) {

      DeclarePodioOutput<edm4eic::Cluster>(GetOutputTags()[0]);
      DeclarePodioOutput<edm4eic::MCRecoClusterParticleAssociation>(GetOutputTags()[1]);

    }

    //------------------------------------------
    // Init
    void Init() override {

        auto app = GetApplication();

        // This prefix will be used for parameters
        std::string param_prefix = GetPluginName() + ":" + GetTag();

        // SpdlogMixin logger initialization, sets m_log
        SpdlogMixin::InitLogger(app, GetPrefix(), "info");

        m_algo.init(logger());
    }

    //------------------------------------------
    // Process
    void Process(const std::shared_ptr<const JEvent> &event) override {

        // TODO: NWB: We are using GetCollectionBase because GetCollection is temporarily out of commission due to JFactoryPodioTFixed
        auto mcparticles = static_cast<const edm4hep::MCParticleCollection*>(event->GetCollectionBase(GetInputTags()[0]));
        auto energy_clusters = static_cast<const edm4eic::ClusterCollection*>(event->GetCollectionBase(GetInputTags()[1]));
        auto energy_assocs = static_cast<const edm4eic::MCRecoClusterParticleAssociationCollection*>(event->GetCollectionBase(GetInputTags()[2]));
        auto position_clusters = static_cast<const edm4eic::ClusterCollection*>(event->GetCollectionBase(GetInputTags()[3]));
        auto position_assocs = static_cast<const edm4eic::MCRecoClusterParticleAssociationCollection*>(event->GetCollectionBase(GetInputTags()[4]));

        try {
            auto clusters = m_algo.process(*mcparticles, *energy_clusters, *energy_assocs, *position_clusters, *position_assocs);
            SetCollection<edm4eic::Cluster>(GetOutputTags()[0], std::move(std::get<0>(clusters)));
            SetCollection<edm4eic::MCRecoClusterParticleAssociation>(GetOutputTags()[1], std::move(std::get<1>(clusters)));
        }
        catch(std::exception &e) {
            throw JException(e.what());
        }

    }

    private:
      eicrecon::TruthEnergyPositionClusterMerger m_algo;

};

} // eicrecon
