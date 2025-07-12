// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#pragma once

#include <ActsExamples/EventData/Track.hpp>
#include <ActsExamples/EventData/Trajectories.hpp>

#include "algorithms/tracking/ActsEdm.h"

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

  using TrackCollection      = std::vector<ActsExamples::ConstTrackContainer*>;
  using TrajectoryCollection = std::vector<ActsExamples::Trajectories*>;
};

#if Acts_VERSION_MAJOR >= 36
static_assert(is_track_container_backend<eicrecon::ActsExamplesEdm>);
static_assert(!is_podio_container<eicrecon::ActsExamplesEdm>);
#endif

} // namespace eicrecon
