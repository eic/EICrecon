// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul, Chun Yuen Tsang, Prithwish Tribedy
// Special Acknowledgement: Kolja Kauder
//
// Convert ADC pulses from SiliconPulseGeneration into ADC and TDC values

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4hep/RawTimeSeriesCollection.h>
#include <string>
#include <string_view>

#include "algorithms/digi/SiliconPulseDigitizationConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using SiliconPulseDigitizationAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::RawTimeSeriesCollection>,
                          algorithms::Output<edm4eic::RawTrackerHitCollection>>;

class SiliconPulseDigitization : public SiliconPulseDigitizationAlgorithm,
                             public WithPodConfig<SiliconPulseDigitizationConfig> {

public:
  SiliconPulseDigitization(std::string_view name)
      : SiliconPulseDigitizationAlgorithm{name, {"SiliconPulse"}, {"ADCTDCOutput"}, {}} {}
  void init(){};
  void process(const Input&, const Output&) const final;
};

} // namespace eicrecon
