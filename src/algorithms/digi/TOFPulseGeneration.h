// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul, Chun Yuen Tsang, Prithwish Tribedy
// Special Acknowledgement: Kolja Kauder
//
// Convert energy deposition into ADC pulses
// ADC pulses are assumed to follow the shape of landau function

#pragma once

#include <algorithms/algorithm.h>
#include <edm4hep/RawTimeSeriesCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <string>
#include <string_view>

#include "algorithms/digi/TOFHitDigiConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using TOFPulseGenerationAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::SimTrackerHitCollection>,
                          algorithms::Output<edm4hep::RawTimeSeriesCollection>>;

class TOFPulseGeneration : public TOFPulseGenerationAlgorithm,
                           public WithPodConfig<TOFHitDigiConfig> {

public:
  TOFPulseGeneration(std::string_view name)
      : TOFPulseGenerationAlgorithm{name, {"TOFBarrelSharedHits"}, {"TOFBarrelPulse"}, {}} {}
  void init(){};
  void process(const Input&, const Output&) const final;

protected:
  double _Landau(double amp, double x, double mean, double std) const;
  double _DigitizeTime(double time) const;
};

} // namespace eicrecon
