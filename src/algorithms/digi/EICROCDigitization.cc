// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul, Chun Yuen Tsang, Prithwish Tribedy
// Special Acknowledgement: Kolja Kauder
//
// Convert ADC pulses into ADC and TDC values

#include <podio/RelationRange.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <limits>
#include <tuple>
#include <vector>

#include "EICROCDigitization.h"
#include "algorithms/digi/EICROCDigitizationConfig.h"
#include "algorithms/digi/SimPulseTrackerHitLinks.h"

namespace eicrecon {

void EICROCDigitization::process(const EICROCDigitization::Input& input,
                                 const EICROCDigitization::Output& output) const {
  const auto [simhits, truths]        = input;
  auto [rawhits, links, associations] = output;

  const bool has_truth = (truths->size() == simhits->size());
  if (!has_truth) {
    error("Waveform truth has {} entries for {} waveforms; raw hits will not be linked",
          truths->size(), simhits->size());
  }

  double thres  = m_cfg.t_thres;
  int adc_range = m_cfg.adc_range;

  for (std::size_t i_pulse = 0; i_pulse < simhits->size(); ++i_pulse) {
    const auto pulse = (*simhits)[i_pulse];
    int tdc          = std::numeric_limits<int>::max();
    int V            = 0;

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
    int adc = std::min(adc_range, -V);
    // only store valid hits
    if (tdc < std::numeric_limits<int>::max()) {
      auto rawhit = rawhits->create(pulse.getCellID(), adc, tdc);
      if (has_truth) {
        addSimPulseTrackerHitLinks(rawhit, (*truths)[i_pulse], *links, *associations);
      }
    }
    //-----------------------------------------------------------
  }
} // EICROCDigitization:process
} // namespace eicrecon
