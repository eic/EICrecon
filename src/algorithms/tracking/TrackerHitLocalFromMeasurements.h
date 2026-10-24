// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#pragma once

#include <ActsPodioEdm/TrackerHitLocalCollection.h>
#include <algorithms/algorithm.h>
#include <edm4eic/Measurement2DCollection.h>
#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using TrackerHitLocalFromMeasurementsAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::Measurement2DCollection>,
                          algorithms::Output<ActsPodioEdm::TrackerHitLocalCollection>>;

/// Converts edm4eic::Measurement2D to ActsPodioEdm::TrackerHitLocal, one-to-one and in the same
/// order, so that the hit at index i corresponds to the measurement at index i. The hits are the
/// targets of the track state to hit links stored with the ActsPodioEdm track state collections.
class TrackerHitLocalFromMeasurements : public TrackerHitLocalFromMeasurementsAlgorithm,
                                        public WithPodConfig<NoConfig> {
public:
  TrackerHitLocalFromMeasurements(std::string_view name)
      : TrackerHitLocalFromMeasurementsAlgorithm{
            name,
            {"inputMeasurements"},
            {"outputTrackerHitsLocal"},
            "convert measurements to sensor-local tracker hits for ACTS"} {}

  void init() final {};
  void process(const Input&, const Output&) const final;
};

} // namespace eicrecon
