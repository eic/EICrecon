// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#include <edm4eic/Cluster.h>
#include <edm4eic/Track.h>
#include <podio/LinkNavigator.h>
#include <map>
#include <tuple>
#include <utility>
#include <vector>

#include "ClusterAttacher.h"
#include "algorithms/interfaces/CompareObjectID.h"
#include "algorithms/interfaces/LinkTruthUtils.h"

namespace eicrecon {

/*! Attach clusters to particles based on the following
 *  algorithm:
 *    1. Build map of tracks onto particles
 *       using particle-track relation,
 *    2. Build map of tracks onto clusters
 *       using track-cluster matches,
 *    3. For each cluster matched to a
 *       track, add it to relevant
 *       particle's cluster relations.
 *
 *  \note If mc-reco links are provided in input, then
 *    they'll be copied over to output.
 */
void ClusterAttacher::process(const ClusterAttacher::Input& input,
                              const ClusterAttacher::Output& output) const {

  // grab inputs/outputs
  const auto [in_particles, in_clusters, in_matches, in_links] = input;
  auto [out_particles, out_links]                              = output;

  // exit if no clusters or matches in input
  if (in_clusters->empty()) {
    debug("No clusters in collection");
    return;
  }
  if (in_matches->empty()) {
    debug("No track-cluster matches in collection");
    return;
  }

  // check if mc links will need to be copied
  const truth::EventLinkNavigator<edm4eic::MCRecoParticleLinkCollection> navigator(in_links);
  const bool copyLinks = navigator.enabled();
  if (!copyLinks) {
    debug("Input MC-reco link collection is empty. Will not copy links.");
  }

  std::map<edm4eic::Track, std::vector<edm4eic::MutableReconstructedParticle>, CompareObjectID<edm4eic::Track>>
      mapTrackToParticles;
  for (const auto& particle : *in_particles) {

    edm4eic::MutableReconstructedParticle clone = particle.clone();
    for (const auto& track : clone.getTracks()) {
      mapTrackToParticles[track].push_back(clone);
    }

    if (copyLinks) {
      const auto linkees = navigator.linked(particle);
      for (const auto& [linkee, weight] : linkees) {
        edm4eic::MutableMCRecoParticleLink link = out_links->create();
        link.setFrom(clone);
        link.setTo(linkee);
        link.setWeight(weight);
      }
    }
  }

  std::map<edm4eic::Track, std::vector<edm4eic::Cluster>, CompareObjectID<edm4eic::Track>>
      mapTrackToClusters;
  for (const auto& match : *in_matches) {
    mapTrackToClusters[match.getTrack()].push_back(match.getCluster());
  }

  for (auto& [track, particles] : mapTrackToParticles) {
    for (auto& particle : particles) {
      for (const auto& cluster : mapTrackToClusters[track]) {
        particle.addToClusters(cluster);
      }
      out_particles->push_back(particle);
    }
  }
} // end 'process(Input&, Output&)'
} // namespace eicrecon
