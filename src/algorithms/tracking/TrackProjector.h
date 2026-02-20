// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Dmitry Romanov, Dmitry Kalinkin

#pragma once

#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/EventData/VectorTrackContainer.hpp>
#include <algorithms/algorithm.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <memory>
#include <string>
#include <string_view>

#include "algorithms/tracking/ActsDD4hepDetector.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using TrackProjectorAlgorithm = algorithms::Algorithm<
    algorithms::Input<Acts::ConstVectorMultiTrajectory, Acts::ConstVectorTrackContainer,
                      edm4eic::TrackCollection>,
    algorithms::Output<edm4eic::TrackSegmentCollection>>;

class TrackProjector : public TrackProjectorAlgorithm, public WithPodConfig<NoConfig> {
public:
  TrackProjector(std::string_view name)
      : TrackProjectorAlgorithm{name,
                                {"inputActsTrackStates", "inputActsTracks", "inputTracks"},
                                {"outputTrackSegments"},
                                "Exports track states as segments"} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  std::shared_ptr<const eicrecon::ActsDD4hepDetector> m_acts_detector;
};

} // namespace eicrecon
