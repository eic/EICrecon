// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul, Chun Yuen Tsang, Prithwish Tribedy
// Special Acknowledgement: Kolja Kauder
//
// Convert ADC pulses from TOFPulseGeneration into ADC and TDC values

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4hep/RawTimeSeriesCollection.h>
#include <string>
#include <string_view>

#include "algorithms/digi/TOFHitDigiConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using TOFPulseDigitizationAlgorithm =
        algorithms::Algorithm<algorithms::Input<edm4hep::RawTimeSeriesCollection>,
                              algorithms::Output<edm4eic::RawTrackerHitCollection>>;

class TOFPulseDigitization : public TOFPulseDigitizationAlgorithm,
                             public WithPodConfig<TOFHitDigiConfig> {

public:
  TOFPulseDigitization(std::string_view name) :
          TOFPulseDigitizationAlgorithm{name,
                                        {"TOFBarrelPulse"},
                                        {"TOFBarrelADCTDC"},
                                        {}} {}
  void init() {};
  void process(const Input&, const Output&) const final;
};

} // namespace eicrecon
