// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Dmitry Kalinkin

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/MCRecoTrackParticleAssociationCollection.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/Measurement2DCollection.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/TrajectoryCollection.h>
#include <fmt/core.h>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "algorithms/tracking/ActsExamplesEdm.h"
#include "algorithms/tracking/ActsPodioEdm.h"

namespace eicrecon {

template <typename edm_t = eicrecon::ActsExamplesEdm>
using ActsToTracksAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::Measurement2DCollection, std::vector<typename edm_t::Trajectories>,
                      std::optional<edm4eic::MCRecoTrackerHitAssociationCollection>>,
    algorithms::Output<edm4eic::TrajectoryCollection, edm4eic::TrackParametersCollection,
                       edm4eic::TrackCollection,
                       std::optional<edm4eic::MCRecoTrackParticleAssociationCollection>>>;

template <typename edm_t = eicrecon::ActsExamplesEdm>
class ActsToTracks : public ActsToTracksAlgorithm<edm_t> {
public:
  ActsToTracks(std::string_view name)
      : ActsToTracksAlgorithm<edm_t>{name,
                                     {
                                         "inputMeasurements",
                                         "inputActsTrajectories",
                                         "inputRawTrackerHitAssociations",
                                     },
                                     {
                                         "outputTrajectories",
                                         "outputTrackParameters",
                                         "outputTracks",
                                         "outputTrackAssociations",
                                     },
                                     "Converts ACTS trajectories to EDM4eic"} {};

  void init() final;
  void process(const typename ActsToTracks<edm_t>::Input&,
               const typename ActsToTracks<edm_t>::Output&) const final;
};

} // namespace eicrecon
