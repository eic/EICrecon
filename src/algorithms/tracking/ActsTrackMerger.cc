// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#include "ActsTrackMerger.h"

#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/EventData/VectorTrackContainer.hpp>
#include <ActsExamples/EventData/Track.hpp>
#include <memory>
#include <vector>

namespace eicrecon {

void ActsTrackMerger::process(const Input& input, const Output& output) const {
  // Note: Output type is const std::tuple<const std::vector<gsl::not_null<T**>>>& which cannot be modified.
  // The factory calls merge() directly instead of using this method.
  // This stub exists only to satisfy the Algorithm interface requirement.
}

std::vector<ActsExamples::ConstTrackContainer*>
ActsTrackMerger::merge(const std::vector<const ActsExamples::ConstTrackContainer*>& input1,
                       const std::vector<const ActsExamples::ConstTrackContainer*>& input2) const {

  std::vector<ActsExamples::ConstTrackContainer*> result;

  // Collect all input track containers
  // Note: std::views::concat becomes available in C++26
  std::vector<const ActsExamples::ConstTrackContainer*> inputs;
  inputs.reserve(input1.size() + input2.size());
  inputs.insert(inputs.end(), input1.begin(), input1.end());
  inputs.insert(inputs.end(), input2.begin(), input2.end());

  if (inputs.empty()) {
    return result;
  }

  // Create new mutable containers for merging
  auto mergedTrackContainer      = std::make_shared<Acts::VectorTrackContainer>();
  auto mergedTrackStateContainer = std::make_shared<Acts::VectorMultiTrajectory>();
  ActsExamples::TrackContainer mergedTracks(mergedTrackContainer, mergedTrackStateContainer);

  // Copy all tracks from all input containers
  for (const auto* inputContainer : inputs) {
    // Ensure dynamic columns exist in merged container
    mergedTracks.ensureDynamicColumns(*inputContainer);

    // Copy each track
    for (const auto& srcTrack : *inputContainer) {
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

  // Create and store the merged ConstTrackContainer
  result.push_back(
      new ActsExamples::ConstTrackContainer(constTrackContainer, constTrackStateContainer));

  return result;
}

} // namespace eicrecon
