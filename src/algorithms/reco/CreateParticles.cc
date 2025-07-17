// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tristan Protzman

#include <edm4eic/EDM4eicVersion.h> // Requires TrackClusterMatchCollection
#if EDM4EIC_VERSION_MAJOR >= 8

#include <edm4eic/ClusterCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/TrackClusterMatchCollection.h>
#include <edm4eic/TrackSegmentCollection.h>

#include "CreateParticles.h"

namespace eicrecon {
void CreateParticles::process(const CreateParticles::Input& input,
                              const CreateParticles::Output& output) const {
                                
  auto [track_segments, clusters, track_cluster_matches] = input;
  auto [reconstructed_particles] = output;
  trace("We have {} track segments, {} clusters, and {} track-cluster matches",
        track_segments->size(), clusters->size(), track_cluster_matches->size());
  }




}



#endif // EDM4EIC_VERSION_MAJOR >= 8