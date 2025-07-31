// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Chun Yuen Tsang
//
// Convert ADC pulses from CFDROCGeneration into ADC and TDC values

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4hep/RawTimeSeriesCollection.h>
#include <cmath>
#include <string>
#include <string_view>

#include "algorithms/digi/CFDROCDigitizationConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using CFDROCDigitizationAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::RawTimeSeriesCollection>,
                          algorithms::Output<edm4eic::RawTrackerHitCollection>>;

class CFDROCDigitization : public CFDROCDigitizationAlgorithm,
                           public WithPodConfig<CFDROCDigitizationConfig> {

public:
  CFDROCDigitization(std::string_view name)
      : CFDROCDigitizationAlgorithm{
            name,
            {"inputWaveforms"},
            {"outputRawHits"},
            "Discriminate and digitize signals with timing based on what is expected from a CFD"} {}
  void init() {};
  void process(const Input&, const Output&) const final;
};

} // namespace eicrecon
