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
  // unsigned int             pedMeanADC{0};
  // double                   pedSigmaADC{0};
  double resolutionTDC{1};
  double resolutionADC{1};

  // Parameters of AC-LGAD signal generation - Added by Souvik
  double gain           = 80;
  double risetime       = 0.45; // 0.02; //in ns
  double sigma_analog   = 0.293951;
  double sigma_sharingx = 0.1;
  double sigma_sharingy = 0.5;
  double Vm             = -1e-4 * gain; // Vm = voltage maximum. When EDep = 1e-4 GeV, voltage corresponds to ADC = adc_max
  double t_thres        = 0.1 * Vm;
  double ignore_thres   = 0.01 * Vm; // If EDep below this value, digitization for the cell will be ignored. Speed up calculation
				     //
  double tMin     = 0.1;
  double tMax     = 100.0;
  int total_time  = ceil(tMax - tMin);
  int time_period = 25;
  int nBins       = 1024;
  int adc_bit     = 8;
  int tdc_bit     = 10;

  int adc_range = pow(2, adc_bit);
  int tdc_range = pow(2, tdc_bit);
};

} // namespace eicrecon
