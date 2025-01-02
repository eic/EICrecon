// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul

#pragma once

#include <DD4hep/DD4hepUnits.h>

namespace eicrecon {

struct LGADPulseGenerationConfig {
  // Parameters of AC-LGAD signal generation
  double gain           = 113.755;
  double risetime       = 0.45 * dd4hep::ns;
  double sigma_analog   = 0.293951 * dd4hep::ns;
  double Vm = -1e-4 * dd4hep::GeV; // Vm = voltage maximum. When EDep = 1e-4 GeV, voltage
                                   // corresponds to ADC = adc_max
  double ignore_thres = 0.001 * Vm; // If EDep below this value, digitization for the cell will be
                                   // ignored. Speed up calculation
  int adc_bit = 8;
  int tdc_bit = 10;

  // total number of TDC/ADC values
  // Since digitizer starts at zero, max ADC value = adc_range - 1
  // Similar for TDC
  int adc_range = std::pow(2, adc_bit);
  int tdc_range = std::pow(2, tdc_bit);

  // period of the sensor clock. Time internal to sensor will all be digitized to integer multiple
  // of tInterval
  double tMax         = 25 * dd4hep::ns; // 25 ns is the period of 40MHz EIC clock
  double tInterval    = tMax / (tdc_range - 1);

};

} // namespace eicrecon
