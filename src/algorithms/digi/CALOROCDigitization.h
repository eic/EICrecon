// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim, Wouter Deconinck, Dmitry Kalinkin, Derek Anderson, Simon Gardner, Sylvester Joosten, Maria Zurek

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/RawCALOROCHitCollection.h>
#include <edm4eic/SimPulseCollection.h>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>

#include "algorithms/digi/CALOROCDigitizationConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using CALOROCDigitizationAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::SimPulseCollection>,
    algorithms::Output<edm4eic::RawCALOROCHitCollection, edm4eic::SimPulseCollection>>;

class CALOROCDigitization : public CALOROCDigitizationAlgorithm,
                            public WithPodConfig<CALOROCDigitizationConfig> {

public:
  CALOROCDigitization(std::string_view name)
      : CALOROCDigitizationAlgorithm{name,
                                     {"InputPulses"},
                                     {"OutputDigiHits", "OutputPulses"},
                                     {"Digitizes simulated pulses from a CALOROC chip"}} {}
  virtual void init() final;
  void process(const Input&, const Output&) const;

private:
  std::function<double(std::uint64_t)> m_toa_thres;
  double get_crossing_time(double thres, double dt, double t, double amp1, double amp2) const;
};

} // namespace eicrecon
