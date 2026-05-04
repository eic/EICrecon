// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/TrackClusterMatchCollection.h>
#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using ChargedCandidateMakerAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::TrackClusterMatchCollection>,
                          algorithms::Output<edm4eic::ReconstructedParticleCollection>>;

// ==========================================================================
// Candidate Charged Particle Maker
// ==========================================================================
/*! An algorithm which takes a collection of track-cluster matches
 *  and converts them into charged-particle candidates, one for
 *  each track.
 */
class ChargedCandidateMaker : public ChargedCandidateMakerAlgorithm,
                              public WithPodConfig<NoConfig> {

public:
  ///! Algorithm constructor
  ChargedCandidateMaker(std::string_view name)
      : ChargedCandidateMakerAlgorithm{name,
                                       {"inputTrackClusterMatches"},
                                       {"outputChargedCandidateParticles"},
                                       "Forms candidate charged particles."} {}

  void process(const Input&, const Output&) const final;

}; // end ChargedCandidateMaker

} // namespace eicrecon
