// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul, Chun Yuen Tsang, Prithwish Tribedy
// Special Acknowledgement: Kolja Kauder
//
// Convert ADC pulses from LGADPulseGeneration into ADC and TDC values

#include <algorithm>
#include <gsl/pointers>
#include <limits>
#include <math.h>
#include <podio/RelationRange.h>
#include <stdlib.h>
#include <vector>

#include "LGADPulseDigitization.h"
#include "algorithms/digi/LGADHitDigiConfig.h"

namespace eicrecon {

void LGADPulseDigitization::process(const LGADPulseDigitization::Input& input,
                                   const LGADPulseDigitization::Output& output) const {
  const auto [simhits] = input;
  auto [rawhits]       = output;

  double thres = m_cfg.t_thres;
  // Vm in unit of GeV. When Edep = Vm, ADC = cfg.adc_range-1
  double Vm     = m_cfg.Vm;
  int adc_range = m_cfg.adc_range;

  // normalized time threshold
  // convert threshold EDep to voltage
  double norm_threshold = -thres * adc_range / Vm;

  for (const auto& pulse : *simhits) {
    double intersectionX = 0.0;
    int tdc              = std::numeric_limits<int>::max();
    int adc              = 0;
    double V             = 0.0;

    int time_bin         = 0;
    double adc_prev      = 0;
    double time_interval = pulse.getInterval();
    auto adcs            = pulse.getAdcCounts();
    double n_EICROC_cycle = int(pulse.getTime()/m_cfg.tMax + 1e-3);
    for (const auto adc : adcs) {
      if (adc_prev >= norm_threshold && adc <= norm_threshold) {
        intersectionX = time_bin * time_interval +
                        time_interval * (norm_threshold - adc_prev) / (adc - adc_prev);
        tdc = static_cast<int>(intersectionX / time_interval) + n_EICROC_cycle * m_cfg.tdc_range;
      }
      if (abs(adc) > abs(V)) // To get peak of the Analog signal
        V = adc;
      adc_prev = adc;
      ++time_bin;
    }

    // limit the range of adc values
    adc = std::min(static_cast<double>(adc_range), round(-V));
    // only store valid hits
    if (tdc < std::numeric_limits<int>::max())
      rawhits->create(pulse.getCellID(), adc, tdc);
    //-----------------------------------------------------------
  }
} // LGADPulseDigitization:process
} // namespace eicrecon
