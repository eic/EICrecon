// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#pragma once

#include <Acts/Plugins/Podio/PodioTrackContainer.hpp>
#include <Acts/Plugins/Podio/PodioTrackStateContainer.hpp>
#include <ActsPodioEdm/TrackCollection.h>
#include <ActsPodioEdm/TrackStateCollection.h>

#include "algorithms/tracking/ActsEdm.h"

namespace eicrecon {

class ActsPodioEdm {

public:
  /// (Reconstructed) track parameters e.g. close to the vertex.
  using TrackParameters = ::Acts::BoundTrackParameters;
  /// Container of reconstructed track states for multiple tracks.
  using TrackParametersContainer = std::vector<TrackParameters>;

  using TrackContainer =
      Acts::TrackContainer<Acts::MutablePodioTrackContainer, Acts::MutablePodioTrackStateContainer,
                           std::shared_ptr>;

  using ConstTrackContainer =
      Acts::TrackContainer<Acts::ConstPodioTrackContainer, Acts::ConstPodioTrackStateContainer,
                           std::shared_ptr>;

  using TrackIndexType = TrackContainer::IndexType;

  using TrackProxy      = TrackContainer::TrackProxy;
  using ConstTrackProxy = ConstTrackContainer::ConstTrackProxy;

  using Trajectories          = ActsExamples::Trajectories;
  using TrajectoriesContainer = ActsExamples::TrajectoriesContainer;

  using TrackCollection      = ::ActsPodioEdm::TrackCollection;
  using TrajectoryCollection = ActsExamples::TrajectoriesContainer;
};

#if Acts_VERSION_MAJOR >= 36
static_assert(is_track_container_backend<eicrecon::ActsPodioEdm>);
static_assert(is_podio_container<eicrecon::ActsPodioEdm>);
#endif

} // namespace eicrecon
