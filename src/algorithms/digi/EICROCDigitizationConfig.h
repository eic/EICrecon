// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul

#pragma once

#include <edm4eic/unit_system.h>

namespace eicrecon {

struct EICROCDigitizationConfig {
  int adc_bit = 8;
  int tdc_bit = 10;
  // total number of TDC/ADC values
  // Since digitizer starts at zero, max ADC value = adc_range - 1
  // Similar for TDC
  int adc_range = std::pow(2, adc_bit);
  int tdc_range = std::pow(2, tdc_bit);

  double t_thres =
      -0.1 *
      adc_range; // TDC value = time when pulse exceed t_thres. Negative because Silicon voltage is negative when hit
  // period of the sensor clock. Time internal to sensor will all be digitized to integer multiple
  // of tInterval
  double tMax = 25 * edm4eic::unit::ns; // 25 ns is the period of 40MHz EIC clock
};

} // namespace eicrecon
