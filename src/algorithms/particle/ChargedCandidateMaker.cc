// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson

#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/Track.h>
#include <fmt/core.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <cmath>
#include <gsl/pointers>
#include <map>
#include <vector>
#if EDM4EIC_VERSION_MAJOR >= 8

#include "ChargedCandidateMaker.h"

namespace eicrecon {

// ----------------------------------------------------------------------------
//! Process inputs
// ----------------------------------------------------------------------------
/*! Construct a candidate charged particle via the
 *  following algorithm.
 *    1. Build map of tracks onto vectors of their
 *       matched tracks
 *    2. For each track, create a Reconstructed
 *       Particle with track and cluster relations
 *       filled
 */
void ChargedCandidateMaker::process(const ChargedCandidateMaker::Input& input,
                                    const ChargedCandidateMaker::Output& output) const {

  // grab inputs/outputs
  const auto [in_match] = input;
  auto [out_particle] = output;

  // exit if no matches in collection
  if (in_match->size() == 0) {
    debug("No track-cluster matches in collection");
    return;
  }

  // --------------------------------------------------------------------------
  // 1. Build map of tracks onto matched clusters
  // --------------------------------------------------------------------------
  MapToVecClust mapTrkToClust;
  for (const auto& match : *in_match) {
    mapTrkToClust[match.getTrack()].push_back(match.getCluster());
  }

  // --------------------------------------------------------------------------
  // 2. Create a reconstructed particle for each track
  // --------------------------------------------------------------------------
  for (const auto& [track, clusters] : mapTrkToClust) {
    edm4eic::MutableReconstructedParticle particle = out_particle->create();
    particle.addToTracks(track);
    for (const edm4eic::Cluster& cluster : clusters) {
      particle.addToClusters(cluster);
    }
  }
} // end 'process(Input&, Output&)'
} // namespace eicrecon

#endif // EDM4EIC_VERSION_MAJOR >= 8
