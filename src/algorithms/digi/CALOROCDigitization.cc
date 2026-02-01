// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim
//

#include <algorithms/service.h>
#include <fmt/core.h>
#include <podio/RelationRange.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <gsl/pointers>
#include <limits>
#include <map>
#include <random>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "CALOROCDigitization.h"

class CALOROCRawSample {
public:
  struct RawEntry {
    double adc{0};
    double toa{0};
    double tot{0};
  };

  CALOROCRawSample(std::size_t n_samps) { rawEntries.resize(n_samps); }

  void setADC(std::size_t idx, double adc) { rawEntries[idx].adc = adc; }
  void setTOA(std::size_t idx, double toa) { rawEntries[idx].toa = toa; }
  void setTOT(std::size_t idx, double tot) { rawEntries[idx].tot = tot; }

  const std::vector<RawEntry>& getEntries() const { return rawEntries; }

private:
  std::vector<RawEntry> rawEntries;
};

namespace eicrecon {

void CALOROCDigitization::init() {}

void CALOROCDigitization::process(const CALOROCDigitization::Input& input,
                                  const CALOROCDigitization::Output& output) const {
  const auto [in_pulses] = input;
  auto [out_digi_hits]   = output;

  for (const auto& pulse : *in_pulses) {
    double pulse_t     = pulse.getTime();
    double pulse_dt    = pulse.getInterval();
    std::size_t n_amps = pulse.getAmplitude().size();

    CALOROCRawSample raw_sample(m_cfg.n_samples);

    // For ADC, amplitudes are measured with a fixed phase.
    // This was reproduced by sample_tick and adc_counter as follows.
    // Amplitude is measured whenever adc_counter reaches sample_tick.
    int sample_tick = static_cast<int>(m_cfg.time_window / pulse_dt);
    std::size_t time_stamp =
        static_cast<std::size_t>(std::ceil((pulse_t - m_cfg.adc_phase) / m_cfg.time_window));
    int adc_counter =
        sample_tick -
        static_cast<int>((m_cfg.adc_phase + time_stamp * m_cfg.time_window - pulse_t) / pulse_dt);
    if (adc_counter < 0 || adc_counter > sample_tick)
      continue;

    std::size_t idx_sample = 0;
    std::size_t idx_toa    = 0;
    double t_upcross       = 0;
    bool tot_progress      = false;

    for (std::size_t i = 0; i < n_amps; i++) {
      double t = pulse_t + i * pulse_dt;

      // Measure amplitudes for ADC
      if (adc_counter == sample_tick) {
        raw_sample.setADC(idx_sample, pulse.getAmplitude()[i]);
        adc_counter = 0;
        idx_sample++;
        if (idx_sample == m_cfg.n_samples)
          break;
      }

      // Measure up-crossing time for TOA
      if (!tot_progress && pulse.getAmplitude()[i] > m_cfg.toa_thres) {
        idx_toa   = idx_sample;
        t_upcross = get_crossing_time(m_cfg.toa_thres, pulse_dt, t, pulse.getAmplitude()[i],
                                      pulse.getAmplitude()[i - 1]);
        raw_sample.setTOA(idx_toa,
                          m_cfg.adc_phase + (time_stamp + idx_toa) * m_cfg.time_window - t_upcross);
        tot_progress = true;
      }

      // Measure down-crossing time for TOT
      if (tot_progress && pulse.getAmplitude()[i] < m_cfg.tot_thres) {
        raw_sample.setTOT(idx_toa,
                          get_crossing_time(m_cfg.tot_thres, pulse_dt, t, pulse.getAmplitude()[i],
                                            pulse.getAmplitude()[i - 1]) -
                              t_upcross);
        tot_progress = false;
      }

      adc_counter++;
    }

    // Fill CALOROCSamples and RawCALOROCHit
    auto out_digi_hit = out_digi_hits->create();
    out_digi_hit.setCellID(pulse.getCellID());
    out_digi_hit.setSamplePhase(std::llround(m_cfg.adc_phase / m_cfg.dyRangeTOA * m_cfg.capTOA));
    out_digi_hit.setTimeStamp(time_stamp);

    const auto& entries = raw_sample.getEntries();

    for (const auto& entry : entries) {
      edm4eic::CALOROC1ASample aSample;
      auto adc = std::max(std::llround(entry.adc / m_cfg.dyRangeSingleGainADC * m_cfg.capADC), 0LL);
      aSample.ADC = adc > m_cfg.capADC ? m_cfg.capADC : adc;
      auto toa    = std::max(std::llround(entry.toa / m_cfg.dyRangeTOA * m_cfg.capTOA), 0LL);
      aSample.timeOfArrival = toa > m_cfg.capTOA ? m_cfg.capTOA : toa;
      auto tot = std::max(std::llround(entry.tot / m_cfg.dyRangeTOT * m_cfg.capTOT), 0LL);
      aSample.timeOverThreshold = tot > m_cfg.capTOT ? m_cfg.capTOT : tot;
      out_digi_hit.addToASamples(aSample);

      edm4eic::CALOROC1BSample bSample;
      auto high_adc =
          std::max(std::llround(entry.adc / m_cfg.dyRangeHighGainADC * m_cfg.capADC), 0LL);
      bool overflow       = high_adc > m_cfg.capADC;
      bSample.highGainADC = overflow ? m_cfg.capADC : high_adc;
      auto low_adc =
          std::max(std::llround(entry.adc / m_cfg.dyRangeLowGainADC * m_cfg.capADC), 0LL);
      bSample.lowGainADC = overflow ? std::min(low_adc, static_cast<long long>(m_cfg.capADC)) : 0LL;
      bSample.timeOfArrival = toa > m_cfg.capTOA ? m_cfg.capTOA : toa;
      out_digi_hit.addToBSamples(bSample);
    }
  }

} // CALOROCDigitization:process

double CALOROCDigitization::get_crossing_time(double thres, double dt, double t, double amp1,
                                              double amp2) const {
  double numerator   = (thres - amp2) * dt;
  double denominator = amp2 - amp1;
  double added       = t;
  return (numerator / denominator) + added;
}
} // namespace eicrecon
