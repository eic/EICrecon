// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024-2025 Simon Gardner, Chun Yuen Tsang, Prithwish Tribedy
//
// Convert energy deposition into analog pulses

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/SimTrackerHitCollection.h>
#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 1)
#include <edm4eic/SimPulseCollection.h>
#else
#include <edm4hep/TimeSeriesCollection.h>
#endif
#include <memory>
#include <string>
#include <string_view>

#include "algorithms/digi/SiliconPulseGenerationConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 1)
using PulseType = edm4eic::SimPulse;
#else
using PulseType = edm4hep::TimeSeries;
#endif

using SiliconPulseGenerationAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::SimTrackerHitCollection>,
                          algorithms::Output<PulseType::collection_type>>;

class SignalPulse;

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
