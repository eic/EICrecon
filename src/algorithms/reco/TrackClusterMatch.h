// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tristan Protzman

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include "algorithms/interfaces/WithPodConfig.h"
#include <spdlog/logger.h>


#include "algorithms/reco/TrackClusterMatchConfig.h"

namespace eicrecon {
    using TrackClusterMatchAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::TrackCollection, edm4eic::ClusterCollection>,
    algorithms::Output<edm4eic::ReconstructedParticleCollection>
    >;
    
    
    class TrackClusterMatch : public TrackClusterMatchAlgorithm, WithPodConfig<TrackClusterMatchConfig> {
    private:
        std::shared_ptr<spdlog::logger> m_log;           

    public:
        TrackClusterMatch(std::string_view name) : 
                TrackClusterMatchAlgorithm{name, {"inputTracks", "inputClusters"}, {"outputParticles"}, ""} {}
        
        void init(std::shared_ptr<spdlog::logger> logger);    
        void execute(const Input&, const Output&) const;    
    };
}