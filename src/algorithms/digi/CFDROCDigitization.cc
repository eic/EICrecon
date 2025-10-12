// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Chun Yuen Tsang
//
// Convert ADC pulses into ADC and TDC values using constant fraction

#include <podio/RelationRange.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <gsl/pointers>
#include <stack>
#include <utility>

#include "CFDROCDigitization.h"
#include "algorithms/digi/CFDROCDigitizationConfig.h"

namespace eicrecon {

void CFDROCDigitization::process(const CFDROCDigitization::Input& input,
                                 const CFDROCDigitization::Output& output) const {
  const auto [simhits] = input;
  auto [rawhits]       = output;

  // The real CFD compares delayed pulse with an inverted scaled pulse
  // This code is doing none of that, it's simply finding pulse height at a fraction of peak
  // more sophisticaed algorithm TBD
  //
  for (const auto& pulse : *simhits) {
    auto adcs = pulse.getAdcCounts();
    if (adcs.size() == 0)
      continue;
    int n_CFDROC_cycle = static_cast<int>(std::floor(pulse.getTime() / m_cfg.tMax));

    // first we find all the peaks and store their location
    // Then we find the time corresponding to fraction of the peak height
    // use stack to store peak height and time
    std::stack<std::pair<int, int>> peakTimeAndHeight;

    for (size_t time_bin = 1; time_bin < adcs.size() - 1; ++time_bin) {
      auto V      = adcs[time_bin];
      auto prev_V = adcs[time_bin - 1];
      auto next_V = adcs[time_bin + 1];
      if ((std::abs(prev_V) < std::abs(V)) &&
          (std::abs(V) >= std::abs(next_V))) { // To get peak of the Analog signal
        peakTimeAndHeight.emplace(time_bin, V);
      }
    }

    // scan the peak in reverse time to find TDC values at fraction of peaks height
    // start from the last adc bin
    int time_bin = static_cast<int>(adcs.size() - 2);
    // find time corresponding to each peak one by one
    while (!peakTimeAndHeight.empty()) {
      auto peak = peakTimeAndHeight.top();
      if (peak.first >= time_bin) {
        // peaks that are situated later than current time are all discarded
        peakTimeAndHeight.pop();
        continue;
      }
      time_bin            = peak.first;
      int target_height_V = static_cast<int>(peak.second * m_cfg.fraction);
      int prev_V          = adcs[time_bin];
      --time_bin;
      // scan the peak in reverse time to find TDC values at fraction of peaks height
      for (; time_bin >= 0; --time_bin) {
        double V = adcs[time_bin];
        // check voltage threshold
        if (std::abs(V) <= std::abs(target_height_V) &&
            std::abs(target_height_V) <= std::abs(prev_V)) {
          int tdc = time_bin + n_CFDROC_cycle * m_cfg.tdc_range;
          // limit the range of adc values
          int adc = std::min(m_cfg.adc_range, std::abs(peak.second));
          rawhits->create(pulse.getCellID(), adc, tdc);
          // the peak is found. Break the time scan to the next peak
          break;
        }
        prev_V = V;
      }
      // discard current peak from stack to look for the next peak
      peakTimeAndHeight.pop();
    }
  }
} // CFDROCDigitization:process
} // namespace eicrecon
