// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#include "ActsTrackMerger.h"

#include <ActsPodioEdm/BoundParametersCollection.h>
#include <ActsPodioEdm/JacobianCollection.h>
#include <ActsPodioEdm/TrackCollection.h>
#include <ActsPodioEdm/TrackStateCollection.h>

namespace eicrecon {

void ActsTrackMerger::process(const Input& input, const Output& output) const {
  const auto [track_states1, track_params1, track_jac1, tracks1, track_states2, track_params2,
              track_jac2, tracks2]                                     = input;
  auto [out_track_states, out_track_params, out_track_jac, out_tracks] = output;

  // Simply copy all objects from both input collections to the output
  // Track states
  for (const auto& ts : *track_states1) {
    out_track_states->push_back(ts.clone());
  }
  for (const auto& ts : *track_states2) {
    out_track_states->push_back(ts.clone());
  }

  // Track parameters
  for (const auto& tp : *track_params1) {
    out_track_params->push_back(tp.clone());
  }
  for (const auto& tp : *track_params2) {
    out_track_params->push_back(tp.clone());
  }

  // Track jacobians
  for (const auto& tj : *track_jac1) {
    out_track_jac->push_back(tj.clone());
  }
  for (const auto& tj : *track_jac2) {
    out_track_jac->push_back(tj.clone());
  }

  // Tracks
  for (const auto& t : *tracks1) {
    out_tracks->push_back(t.clone());
  }
  for (const auto& t : *tracks2) {
    out_tracks->push_back(t.clone());
  }
}

} // namespace eicrecon
