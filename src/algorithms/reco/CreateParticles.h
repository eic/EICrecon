// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tristan Protzman

#pragma once

#include <algorithms/algorithm.h>
#include <algorithms/interfaces/WithPodConfig.h>

#include <edm4eic/ClusterCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/TrackClusterMatchCollection.h>
#include <edm4eic/TrackSegmentCollection.h>

#include "algorithms/reco/CreateParticlesConfig.h"

namespace eicrecon {
using CreateParticlesAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::TrackSegmentCollection, edm4eic::ClusterCollection, edm4eic::TrackClusterMatchCollection>,
    algorithms::Output<edm4eic::ReconstructedParticleCollection>>;

class CreateParticles : public CreateParticlesAlgorithm,
                        public WithPodConfig<CreateParticlesConfig> {

public:
    CreateParticles(std::string_view name) : CreateParticlesAlgorithm {
        name, {"inputTrackSegments", "inputClusters", "inputTrackClusterMatches"},
        {"outputReconstructedParticles"}, ""} {}
    
    void init() final {}
    void process(const Input& input, const Output& output) const final;
};
}