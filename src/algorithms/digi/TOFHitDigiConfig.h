// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul

#pragma once

#include <DD4hep/DD4hepUnits.h>

namespace eicrecon {

struct TOFHitDigiConfig {
  // single hit energy deposition threshold
  double threshold{1.0 * dd4hep::keV};
  double tRes = 0.1; /// TODO 8 of what units??? Same TODO in juggler. Probably [ns]
                     // digitization settings
  double resolutionTDC{1};
  double resolutionADC{1};

  // Parameters of AC-LGAD signal generation
  double gain           = 113.755;
  double risetime       = 0.45; //in ns
  double sigma_analog   = 0.293951;
  double sigma_sharingx = 0.1;
  double sigma_sharingy = 0.5;
  double Vm             = -1e-4 * dd4hep::GeV; // Vm = voltage maximum. When EDep = 1e-4 GeV, voltage corresponds to ADC = adc_max
  double t_thres        = 0.1 * Vm;
  double ignore_thres   = 0.01 * Vm; // If EDep below this value, digitization for the cell will be ignored. Speed up calculation
                                     //
  double tMin     = 0.1;
  double tMax     = 25;// 25 ns is the period of 40MHz EIC clock
  int total_time  = ceil(tMax - tMin);
  int adc_bit     = 8;
  int tdc_bit     = 10;

  int adc_range = pow(2, adc_bit);
  int tdc_range = pow(2, tdc_bit);

  std::string readout = "TOFBarrelHits";
};

} // namespace eicrecon
