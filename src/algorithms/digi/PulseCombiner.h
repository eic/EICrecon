// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Simon Gardner
//
// Convert energy deposition into ADC pulses
// ADC pulses are assumed to follow the shape of landau function

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/SimPulseCollection.h>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "algorithms/digi/PulseCombinerConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using PulseType = edm4eic::SimPulse;

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
