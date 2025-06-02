// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul, Chun Yuen Tsang, Prithwish Tribedy
// Special Acknowledgement: Kolja Kauder
//
// Convert ADC pulses into ADC and TDC values

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <gsl/pointers>
#include <limits>
#include <podio/RelationRange.h>
#include <vector>

#include "EICROCDigitization.h"
#include "algorithms/digi/EICROCDigitizationConfig.h"

namespace eicrecon {

void EICROCDigitization::process(const EICROCDigitization::Input& input,
                                 const EICROCDigitization::Output& output) const {
  const auto [simhits] = input;
  auto [rawhits]       = output;

  double thres  = m_cfg.t_thres;
  int adc_range = m_cfg.adc_range;

  for (const auto& pulse : *simhits) {
    int tdc  = std::numeric_limits<int>::max();
    int adc  = 0;
    double V = 0.0;

    int time_bin          = 0;
    double adc_prev       = 0;
    auto adcs             = pulse.getAdcCounts();
    double n_EICROC_cycle = static_cast<int>(std::floor(pulse.getTime() / m_cfg.tMax + 1e-3));
    for (const auto adc : adcs) {
      if (adc_prev >= thres && adc <= thres) {
        tdc = time_bin + n_EICROC_cycle * m_cfg.tdc_range;
      }
      if (std::abs(adc) > std::abs(V)) { // To get peak of the Analog signal
        V = adc;
      }
      adc_prev = adc;
      ++time_bin;
    }

    // limit the range of adc values
    adc = std::min(static_cast<double>(adc_range), std::round(-V));
    // only store valid hits
    if (tdc < std::numeric_limits<int>::max()) {
      rawhits->create(pulse.getCellID(), adc, tdc);
    }
    //-----------------------------------------------------------
  }
} // EICROCDigitization:process
} // namespace eicrecon
