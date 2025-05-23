// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Simon Gardner
//
// Convert energy deposition into ADC pulses
// ADC pulses are assumed to follow the shape of landau function

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/EDM4eicVersion.h>
#include <cstdint>
#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 1)
#include <edm4eic/SimPulseCollection.h>
#else
#include <edm4hep/TimeSeriesCollection.h>
#endif
#include <string>
#include <string_view>
#include <vector>

#include "algorithms/digi/PulseCombinerConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 1)
using PulseType = edm4eic::SimPulse;
#else
using PulseType = edm4hep::TimeSeries;
#endif

using PulseCombinerAlgorithm =
    algorithms::Algorithm<algorithms::Input<PulseType::collection_type>,
                          algorithms::Output<PulseType::collection_type>>;

class PulseCombiner : public PulseCombinerAlgorithm, public WithPodConfig<PulseCombinerConfig> {

public:
  PulseCombiner(std::string_view name)
      : PulseCombinerAlgorithm{name, {"InputPulses"}, {"OutputPulses"}, {}} {}
  virtual void init() final;
  void process(const Input&, const Output&) const final;

private:
  std::vector<std::vector<PulseType>> clusterPulses(const std::vector<PulseType> pulses) const;
  static std::vector<float> sumPulses(const std::vector<PulseType> pulses);
  uint64_t m_detector_bitmask = 0xFFFFFFFFFFFFFFFF;
};

} // namespace eicrecon
