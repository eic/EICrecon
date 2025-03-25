// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tristan Protzman

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/TrackClusterMatchCollection.h>
#include "algorithms/interfaces/WithPodConfig.h"
#include <spdlog/logger.h>
#include <DD4hep/Detector.h>
#include <edm4hep/Vector3f.h>


#include "algorithms/reco/TrackClusterMatchConfig.h"

namespace eicrecon {
    using TrackClusterMatchAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::TrackSegmentCollection, edm4eic::ClusterCollection>,
    algorithms::Output<edm4eic::TrackClusterMatchCollection>
    >;


    class TrackClusterMatch : public TrackClusterMatchAlgorithm, WithPodConfig<TrackClusterMatchConfig> {
    private:
        std::shared_ptr<spdlog::logger> m_log;
        const dd4hep::Detector* m_detector;
        double distance(const edm4hep::Vector3f& v1, const edm4hep::Vector3f& v2) const;


    public:
        TrackClusterMatch(std::string_view name) :
                TrackClusterMatchAlgorithm{name, {"inputTracks", "inputClusters"}, {"outputParticles"}, ""} {}

        void init(std::shared_ptr<spdlog::logger> logger, const dd4hep::Detector* detector);
        void execute(const Input&, const Output&) const;
    };
}
