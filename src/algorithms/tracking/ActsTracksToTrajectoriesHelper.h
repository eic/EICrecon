// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#pragma once

#include <Acts/EventData/ProxyAccessor.hpp>
#include <ActsExamples/EventData/Track.hpp>
#include <ActsExamples/EventData/Trajectories.hpp>

namespace eicrecon {

inline ActsExamples::TrajectoriesContainer CreateTrajectories(const ActsExamples::ConstTrackContainer& tracks) {

  // Seed number column accessor
  const Acts::ConstProxyAccessor<unsigned int> constSeedNumber("seed");

  // Prepare the output data with MultiTrajectory, per seed
  ActsExamples::TrajectoriesContainer trajectories;
  trajectories.reserve(tracks.size());

  ActsExamples::Trajectories::IndexedParameters parameters;
  std::vector<Acts::MultiTrajectoryTraits::IndexType> tips;

  std::optional<unsigned int> lastSeed;
  for (const auto& track : tracks) {
    if (!lastSeed) {
      lastSeed = constSeedNumber(track);
    }

    if (constSeedNumber(track) != lastSeed.value()) {
      // make copies and clear vectors
      trajectories.emplace_back(tracks.trackStateContainer(), tips, parameters);

      tips.clear();
      parameters.clear();
    }

    lastSeed = constSeedNumber(track);

    tips.push_back(track.tipIndex());
    parameters.emplace(std::make_pair(
        track.tipIndex(), 
        ActsExamples::TrackParameters(track.referenceSurface().getSharedPtr(), track.parameters(),
                                      track.covariance(), track.particleHypothesis())));
  }

  if (tips.empty()) {
    // FIXME logger not defined
    //ACTS_DEBUG("Last trajectory is empty");
  }

  // last entry: move vectors
  trajectories.emplace_back(tracks.trackStateContainer(), std::move(tips), std::move(parameters));

  return trajectories;
}

} // namespace eicrecon
