// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/MCRecoParticleLinkCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/TrackClusterMatchCollection.h>
#include <optional>
#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using ClusterAttacherAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::ReconstructedParticleCollection,
                                            edm4eic::ClusterCollection,
                                            edm4eic::TrackClusterMatchCollection,
                                            std::optional<edm4eic::MCRecoParticleLinkCollection>>,
                          algorithms::Output<edm4eic::ReconstructedParticleCollection,
                                             std::optional<edm4eic::MCRecoParticleLinkCollection>>>;

// ==========================================================================
// Cluster Attacher
// ==========================================================================
/*! An algorithm to attach clusters to particles
 *  based on a collection of track-cluster matches.
 */
class ClusterAttacher : public ClusterAttacherAlgorithm,
                        public WithPodConfig<NoConfig> {

public:
  ClusterAttacher(std::string_view name)
      : ClusterAttacherAlgorithm{name,
                                 {"inputParticles",
                                  "inputClusters",
                                  "inputTrackClusterMatches",
                                  "optionalInputParticleLinks"},
                                 {"outputParticles",
                                  "optionalOutputParticleLinks"},
                                 "Attach clusters to particles."} {}

  void process(const Input&, const Output&) const final;

}; // end ClusterAttacher

} // namespace eicrecon
