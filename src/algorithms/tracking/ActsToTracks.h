// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Dmitry Kalinkin

#pragma once

#include <ActsExamples/EventData/Track.hpp>
#include <ActsExamples/EventData/Trajectories.hpp>
#include <algorithms/algorithm.h>
#include <edm4eic/Measurement2DCollection.h>
#include <edm4eic/TrajectoryCollection.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/TrackCollection.h>

namespace eicrecon {

using ActsToTracksAlgorithm =
    algorithms::Algorithm<
      algorithms::Input<edm4eic::Measurement2DCollection, std::vector<ActsExamples::Trajectories>>,
      algorithms::Output<edm4eic::TrajectoryCollection, edm4eic::TrackParametersCollection, edm4eic::TrackCollection>
    >;

class ActsToTracks : public ActsToTracksAlgorithm {
public:
    ActsToTracks(std::string_view name) : ActsToTracksAlgorithm{name, {"inputActsTrajectories", "inputActsConstTrackContainer"}, {"outputTrajectoryCollection", "outputTrackParametersCollection", "outputTrackCollection"}, "Converts ACTS trajectories to EDM4eic"} {};

    void init() final;
    void process(const Input&, const Output&) const final;
};

}
