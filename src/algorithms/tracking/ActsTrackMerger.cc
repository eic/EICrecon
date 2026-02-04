// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#include "ActsTrackMerger.h"

#include <Acts/EventData/MeasurementHelpers.hpp>
#include <Acts/EventData/TrackContainer.hpp>
#include <Acts/EventData/TrackProxy.hpp>
#include <Acts/EventData/TrackStatePropMask.hpp>
#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/EventData/VectorTrackContainer.hpp>
#include <ActsExamples/EventData/Track.hpp>
#include <Eigen/LU> // IWYU pragma: keep
#include <any>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace eicrecon {

void ActsTrackMerger::process(const Input& input, const Output& output) const {
  const auto [input_track_states1, input_tracks1, input_track_states2, input_tracks2] = input;
  auto [output_track_states, output_tracks]                                           = output;

  // Collect all input track containers by reconstructing ConstTrackContainer wrappers
  std::vector<ActsExamples::ConstTrackContainer> input_containers;

  // Process first input set
  auto trackStateContainer1 =
      std::make_shared<Acts::ConstVectorMultiTrajectory>(*input_track_states1);
  auto trackContainer1 = std::make_shared<Acts::ConstVectorTrackContainer>(*input_tracks1);
  input_containers.emplace_back(trackContainer1, trackStateContainer1);

  // Process second input set
  auto trackStateContainer2 =
      std::make_shared<Acts::ConstVectorMultiTrajectory>(*input_track_states2);
  auto trackContainer2 = std::make_shared<Acts::ConstVectorTrackContainer>(*input_tracks2);
  input_containers.emplace_back(trackContainer2, trackStateContainer2);

  // Create new mutable containers for merging
  auto mergedTrackContainer      = std::make_shared<Acts::VectorTrackContainer>();
  auto mergedTrackStateContainer = std::make_shared<Acts::VectorMultiTrajectory>();
  ActsExamples::TrackContainer mergedTracks(mergedTrackContainer, mergedTrackStateContainer);

  // Copy all tracks from all input containers
  for (const auto& inputContainer : input_containers) {
    // Ensure dynamic columns exist in merged container
    mergedTracks.ensureDynamicColumns(inputContainer);

    // Copy each track
    for (const auto& srcTrack : inputContainer) {
      auto destTrack = mergedTracks.makeTrack();
      destTrack.copyFrom(srcTrack);
    }
  }

  // Allocate new const containers and assign pointers to outputs
  // (Cannot use placement new because Output<T>::Reset() clears the vector)
  *output_track_states =
      new Acts::ConstVectorMultiTrajectory(std::move(*mergedTrackStateContainer));
  *output_tracks = new Acts::ConstVectorTrackContainer(std::move(*mergedTrackContainer));
}

} // namespace eicrecon
