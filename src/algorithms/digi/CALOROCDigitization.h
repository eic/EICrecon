// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/SimPulseCollection.h>
#include <edm4eic/RawCALOROCHitCollection.h>
#include <edm4eic/CALOROC1ASample.h>
#include <edm4eic/CALOROC1BSample.h>
#include <stdint.h>
#include <functional>
#include <string>
#include <string_view>

#include "algorithms/digi/CALOROCDigitizationConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using CALOROCDigitizationAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::SimPulseCollection>,
                          algorithms::Output<edm4eic::RawCALOROCHitCollection>>;

class CALOROCDigitization : public CALOROCDigitizationAlgorithm, public WithPodConfig<CALOROCDigitizationConfig> {

public:
  CALOROCDigitization(std::string_view name)
      : CALOROCDigitizationAlgorithm{name,
                           {"InputPulses"},
                           {"OutputDigiHits"},
                           {"Digitizes simulated pulses from a CALOROC chip"}} {}
  virtual void init() final;
  void process(const Input&, const Output&) const;

private:
  double get_crossing_time(double thres, double dt, double t, double amp1, double amp2) const;
};

} // namespace eicrecon
