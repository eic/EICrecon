// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#pragma once

#include <ActsExamples/EventData/Track.hpp>
#include <ActsExamples/EventData/Trajectories.hpp>

namespace eicrecon {

class ActsExamplesEdm {

public:
  /// (Reconstructed) track parameters e.g. close to the vertex.
  using TrackParameters = ActsExamples::TrackParameters;
  /// Container of reconstructed track states for multiple tracks.
  using TrackParametersContainer = ActsExamples::TrackParametersContainer;

  using TrackContainer = ActsExamples::TrackContainer;

  using ConstTrackContainer = ActsExamples::ConstTrackContainer;

  using TrackIndexType = ActsExamples::TrackIndexType;

  using TrackProxy      = ActsExamples::TrackProxy;
  using ConstTrackProxy = ActsExamples::ConstTrackProxy;

  using Trajectories          = ActsExamples::Trajectories;
  using TrajectoriesContainer = ActsExamples::TrajectoriesContainer;
};

} // namespace eicrecon
