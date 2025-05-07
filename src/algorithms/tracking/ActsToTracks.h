// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Dmitry Kalinkin

#pragma once

#include <algorithms/algorithm.h>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace ActsExamples {
struct Trajectories;
}
namespace edm4eic {
class MCRecoTrackParticleAssociationCollection;
}
namespace edm4eic {
class MCRecoTrackerHitAssociationCollection;
}
namespace edm4eic {
class Measurement2DCollection;
}
namespace edm4eic {
class TrackCollection;
}
namespace edm4eic {
class TrackParametersCollection;
}
namespace edm4eic {
class TrajectoryCollection;
}

namespace eicrecon {

using ActsToTracksAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::Measurement2DCollection, std::vector<ActsExamples::Trajectories>,
                      std::optional<edm4eic::MCRecoTrackerHitAssociationCollection>>,
    algorithms::Output<edm4eic::TrajectoryCollection, edm4eic::TrackParametersCollection,
                       edm4eic::TrackCollection,
                       std::optional<edm4eic::MCRecoTrackParticleAssociationCollection>>>;

class ActsToTracks : public ActsToTracksAlgorithm {
public:
  ActsToTracks(std::string_view name)
      : ActsToTracksAlgorithm{name,
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
  void process(const Input&, const Output&) const final;
};

} // namespace eicrecon
