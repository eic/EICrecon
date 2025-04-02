// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024-2025 Simon Gardner, Chun Yuen Tsang, Prithwish Tribedy
//
// Convert energy deposition into analog pulses

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <edm4hep/TimeSeriesCollection.h>
#include <memory>
#include <string>
#include <string_view>

#include "algorithms/digi/SiliconPulseGenerationConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

class SignalPulse;

using SiliconPulseGenerationAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::SimTrackerHitCollection>,
                          algorithms::Output<edm4hep::TimeSeriesCollection>>;

class SiliconPulseGeneration : public SiliconPulseGenerationAlgorithm,
                               public WithPodConfig<SiliconPulseGenerationConfig> {

public:
  SiliconPulseGeneration(std::string_view name)
      : SiliconPulseGenerationAlgorithm{name, {"RawHits"}, {"OutputPulses"}, {}} {}
  void init() final;
  void process(const Input&, const Output&) const final;

private:

  std::shared_ptr<SignalPulse> m_pulse;
  float m_min_sampling_time = 0 * edm4eic::unit::ns;

};

} // namespace eicrecon
