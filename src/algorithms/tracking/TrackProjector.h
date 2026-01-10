// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Dmitry Romanov, Dmitry Kalinkin

#pragma once

#include <ActsExamples/EventData/Track.hpp>
#include <algorithms/algorithm.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "ActsGeometryProvider.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using TrackProjectorAlgorithm = algorithms::Algorithm<
    algorithms::Input<ActsExamples::ConstTrackContainer, edm4eic::TrackCollection>,
    algorithms::Output<edm4eic::TrackSegmentCollection>>;

class TrackProjector : public TrackProjectorAlgorithm, public WithPodConfig<NoConfig> {
public:
  TrackProjector(std::string_view name)
      : TrackProjectorAlgorithm{name,
                                {"inputActsTracks", "inputTracks"},
                                {"outputTrackSegments"},
                                "Exports track states as segments"} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  std::shared_ptr<const ActsGeometryProvider> m_geo_provider;
};

} // namespace eicrecon
