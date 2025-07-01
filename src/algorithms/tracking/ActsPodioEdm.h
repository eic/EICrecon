// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#pragma once

#include <Acts/Plugins/Podio/PodioTrackContainer.hpp>
#include <Acts/Plugins/Podio/PodioTrackStateContainer.hpp>

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
};

} // namespace eicrecon
