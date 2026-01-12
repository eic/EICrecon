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
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace eicrecon {

void ActsTrackMerger::process(const Input& /* input */, const Output& /* output */) const {
  // This algorithm is intentionally not wired through the standard Algorithm::process
  // mechanism. The factory calls merge() directly with concrete input/output types.
  // If process() is ever invoked, treat it as a misuse and fail fast.
  throw std::logic_error(
      "ActsTrackMerger::process() is not implemented; the factory must call merge() directly.");
}

std::tuple<std::vector<Acts::ConstVectorMultiTrajectory*>,
           std::vector<Acts::ConstVectorTrackContainer*>>
ActsTrackMerger::merge(
    const std::vector<const Acts::ConstVectorMultiTrajectory*>& input_track_states1,
    const std::vector<const Acts::ConstVectorTrackContainer*>& input_tracks1,
    const std::vector<const Acts::ConstVectorMultiTrajectory*>& input_track_states2,
    const std::vector<const Acts::ConstVectorTrackContainer*>& input_tracks2) {

  std::vector<Acts::ConstVectorMultiTrajectory*> result_track_states;
  std::vector<Acts::ConstVectorTrackContainer*> result_tracks;

  // Collect all input track containers by reconstructing ConstTrackContainer wrappers
  std::vector<ActsExamples::ConstTrackContainer> input_containers;

  // Process first input set
  for (size_t i = 0; i < input_track_states1.size() && i < input_tracks1.size(); ++i) {
    auto trackStateContainer =
        std::make_shared<Acts::ConstVectorMultiTrajectory>(*input_track_states1[i]);
    auto trackContainer = std::make_shared<Acts::ConstVectorTrackContainer>(*input_tracks1[i]);
    input_containers.emplace_back(trackContainer, trackStateContainer);
  }

  // Process second input set
  for (size_t i = 0; i < input_track_states2.size() && i < input_tracks2.size(); ++i) {
    auto trackStateContainer =
        std::make_shared<Acts::ConstVectorMultiTrajectory>(*input_track_states2[i]);
    auto trackContainer = std::make_shared<Acts::ConstVectorTrackContainer>(*input_tracks2[i]);
    input_containers.emplace_back(trackContainer, trackStateContainer);
  }

  // Create new mutable containers for merging (even if inputs are empty)
  auto mergedTrackContainer      = std::make_shared<Acts::VectorTrackContainer>();
  auto mergedTrackStateContainer = std::make_shared<Acts::VectorMultiTrajectory>();
  ActsExamples::TrackContainer mergedTracks(mergedTrackContainer, mergedTrackStateContainer);

  // Copy all tracks from all input containers
  for (const auto& inputContainer : input_containers) {
    // Ensure dynamic columns exist in merged container
    mergedTracks.ensureDynamicColumns(inputContainer);

    // Copy each track
    for (const auto& srcTrack : inputContainer) {
      auto destTrack = mergedTracks.getTrack(mergedTracks.addTrack());
#if Acts_VERSION_MAJOR < 43 || (Acts_VERSION_MAJOR == 43 && Acts_VERSION_MINOR < 2)
      destTrack.copyFrom(srcTrack, true); // true = copy track states
#else
      destTrack.copyFrom(srcTrack);
#endif
    }
  }

  // Convert to const containers
  auto constTrackStateContainer =
      std::make_shared<Acts::ConstVectorMultiTrajectory>(std::move(*mergedTrackStateContainer));
  auto constTrackContainer =
      std::make_shared<Acts::ConstVectorTrackContainer>(std::move(*mergedTrackContainer));

  // Create and store the merged containers
  result_track_states.push_back(new Acts::ConstVectorMultiTrajectory(*constTrackStateContainer));
  result_tracks.push_back(new Acts::ConstVectorTrackContainer(*constTrackContainer));

  return {result_track_states, result_tracks};
}

} // namespace eicrecon
