// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Chun Yuen Tsang
#pragma once

#include <edm4eic/unit_system.h>

namespace eicrecon {

struct CFDROCDigitizationConfig {
  int adc_bit = 8;
  int tdc_bit = 10;
  // total number of TDC/ADC values
  // Since digitizer starts at zero, max ADC value = adc_range - 1
  // Similar for TDC
  int adc_range = std::pow(2, adc_bit) * 10;
  int tdc_range = std::pow(2, tdc_bit);

  double fraction = 0.5;
  double tMax     = 25 * edm4eic::unit::ns; // 25 ns is the period of 40MHz EIC clock
};

} // namespace eicrecon
