// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul, Chun Yuen Tsang, Prithwish Tribedy
// Special Acknowledgement: Kolja Kauder
//
// Convert ADC pulses from EICROCGeneration into ADC and TDC values

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4hep/RawTimeSeriesCollection.h>
#include <cmath>
#include <string>
#include <string_view>

#include "algorithms/digi/EICROCDigitizationConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using EICROCDigitizationAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::RawTimeSeriesCollection>,
                          algorithms::Output<edm4eic::RawTrackerHitCollection>>;

class EICROCDigitization : public EICROCDigitizationAlgorithm,
                           public WithPodConfig<EICROCDigitizationConfig> {

public:
  EICROCDigitization(std::string_view name)
      : EICROCDigitizationAlgorithm{name, {"EICROC"}, {"ADCTDCOutput"}, {}} {}
  void init() {};
  void process(const Input&, const Output&) const final;
};

} // namespace eicrecon
