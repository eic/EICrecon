// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul, Chun Yuen Tsang, Prithwish Tribedy
// Special Acknowledgement: Kolja Kauder
//
// Convert ADC pulses from TOFPulseGeneration into ADC and TDC values
//
// Author: Souvik Paul, Chun Yuen Tsang
// Date: 22/10/2024

#pragma once

#include "TF1.h"
#include <iostream>
#include <memory>
#include <random>
#include <vector>

#include <DD4hep/Detector.h>
#include <algorithms/algorithm.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4hep/RawTimeSeriesCollection.h>
#include <spdlog/spdlog.h>

#include "DD4hep/Detector.h"
#include "DDRec/Surface.h"
#include <DDRec/CellIDPositionConverter.h>

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
