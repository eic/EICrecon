// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul, Chun Yuen Tsang, Prithwish Tribedy
// Special Acknowledgement: Kolja Kauder
//
// Convert ADC pulses from LGADPulseGeneration into ADC and TDC values

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4hep/RawTimeSeriesCollection.h>
#include <string>
#include <string_view>

#include "algorithms/digi/LGADPulseDigitizationConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using LGADPulseDigitizationAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::RawTimeSeriesCollection>,
                          algorithms::Output<edm4eic::RawTrackerHitCollection>>;

class LGADPulseDigitization : public LGADPulseDigitizationAlgorithm,
                             public WithPodConfig<LGADPulseDigitizationConfig> {

public:
  LGADPulseDigitization(std::string_view name)
      : LGADPulseDigitizationAlgorithm{name, {"LGADPulse"}, {"ADCTDCOutput"}, {}} {}
  void init(){};
  void process(const Input&, const Output&) const final;
};

} // namespace eicrecon
