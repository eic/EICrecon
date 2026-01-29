// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Dmitry Kalinkin

#pragma once

#include <ActsExamples/EventData/Track.hpp>
#include <algorithms/algorithm.h>
#include <edm4eic/MCRecoTrackParticleAssociationCollection.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/Measurement2DCollection.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/TrackSeedCollection.h>
#include <edm4eic/TrajectoryCollection.h>
#include <optional>
#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"

#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
#include <edm4eic/MCRecoTrackParticleLinkCollection.h>
#endif

namespace eicrecon {

using ActsToTracksAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::Measurement2DCollection, edm4eic::TrackSeedCollection,
                      ActsExamples::ConstTrackContainer,
                      std::optional<edm4eic::MCRecoTrackerHitAssociationCollection>>,
    algorithms::Output<edm4eic::TrajectoryCollection, edm4eic::TrackParametersCollection,
                       edm4eic::TrackCollection,
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                       std::optional<edm4eic::MCRecoTrackParticleLinkCollection>,
#endif
                       std::optional<edm4eic::MCRecoTrackParticleAssociationCollection>>>;

class ActsToTracks : public ActsToTracksAlgorithm, public WithPodConfig<NoConfig> {
public:
  ActsToTracks(std::string_view name)
      : ActsToTracksAlgorithm{name,
                              {
                                  "inputMeasurements",
                                  "inputTrackSeeds",
                                  "inputActsTracks",
                                  "inputRawTrackerHitAssociations",
                              },
                              {
                                  "outputTrajectories",
                                  "outputTrackParameters",
                                  "outputTracks",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                  "outputTrackLinks",
#endif
                                  "outputTrackAssociations",
                              },
                              "Converts ACTS tracks to EDM4eic"} {};

  void init() final;
  void process(const Input&, const Output&) const final;
};

} // namespace eicrecon
