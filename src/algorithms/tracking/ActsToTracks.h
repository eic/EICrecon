// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Dmitry Kalinkin

#pragma once

#include <ActsExamples/EventData/Track.hpp>
#include <algorithms/algorithm.h>
#include <edm4eic/MCRecoTrackParticleAssociationCollection.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/Measurement2DCollection.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/TrackSeedCollection.h>
#include <edm4eic/TrajectoryCollection.h>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using ActsToTracksAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::Measurement2DCollection, ActsExamples::ConstTrackContainer,
                      std::optional<edm4eic::MCRecoTrackerHitAssociationCollection>>,
    algorithms::Output<edm4eic::TrajectoryCollection, edm4eic::TrackParametersCollection,
                       edm4eic::TrackCollection,
                       std::optional<edm4eic::MCRecoTrackParticleAssociationCollection>>>;

class ActsToTracks : public ActsToTracksAlgorithm, public WithPodConfig<NoConfig> {
public:
  ActsToTracks(std::string_view name)
      : ActsToTracksAlgorithm{name,
                              {
                                  "inputMeasurements",
                                  "inputActsTracks",
                                  "inputRawTrackerHitAssociations",
                              },
                              {
                                  "outputTrajectories",
                                  "outputTrackParameters",
                                  "outputTracks",
                                  "outputTrackAssociations",
                              },
                              "Converts ACTS tracks to EDM4eic"} {};

  void init() final;
  void process(const Input&, const Output&) const final;
};

} // namespace eicrecon
