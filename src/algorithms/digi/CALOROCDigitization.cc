// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim
//

#include <edm4eic/EDM4eicVersion.h>

#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 7)
#include <edm4eic/CALOROC1ASample.h>
#include <edm4eic/CALOROC1BSample.h>
#include <podio/RelationRange.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <gsl/pointers>
#include <vector>

#include "CALOROCDigitization.h"

namespace {

struct RawEntry {
  double adc{0};
  double toa{0};
  double tot{0};
};
} // namespace

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
    std::size_t time_stamp =
        static_cast<std::size_t>(std::ceil((pulse_t - m_cfg.adc_phase) / m_cfg.time_window));
    std::size_t idx_amp_first = static_cast<std::size_t>(
        (m_cfg.adc_phase + time_stamp * m_cfg.time_window - pulse_t) / pulse_dt);
    std::size_t sample_tick = static_cast<std::size_t>(m_cfg.time_window / pulse_dt);

    std::vector<RawEntry> raw_samples(m_cfg.n_samples);

    // ADCs are filled in advance because the measurement indices
    // are already determined.
    for (std::size_t i = 0; i < m_cfg.n_samples; i++) {
      std::size_t idx_amp = idx_amp_first + i * sample_tick;
      if (idx_amp < n_amps)
        raw_samples[i].adc = pulse.getAmplitude()[idx_amp];
      else
        break;
    }

    std::size_t idx_sample = 0;
    std::size_t idx_toa    = 0;
    double t_upcross       = 0;
    bool tot_progress      = false;

    // Measure the TOAs and TOTs while scanning the amplitudes.
    for (std::size_t i = 1; i < n_amps; i++) {
      double t = pulse_t + i * pulse_dt;
      if (i > idx_amp_first)
        idx_sample = (i + sample_tick - idx_amp_first - 1) / sample_tick;
      if (idx_sample == m_cfg.n_samples)
        break;

      // Measure up-crossing time for TOA
      if (!tot_progress && pulse.getAmplitude()[i] > m_cfg.toa_thres) {
        idx_toa   = idx_sample;
        t_upcross = get_crossing_time(m_cfg.toa_thres, pulse_dt, t, pulse.getAmplitude()[i],
                                      pulse.getAmplitude()[i - 1]);
        raw_samples[idx_toa].toa =
            m_cfg.adc_phase + (time_stamp + idx_toa) * m_cfg.time_window - t_upcross;
        tot_progress = true;
      }

      // Measure down-crossing time for TOT
      if (tot_progress && pulse.getAmplitude()[i] < m_cfg.tot_thres) {
        raw_samples[idx_toa].tot =
            get_crossing_time(m_cfg.tot_thres, pulse_dt, t, pulse.getAmplitude()[i],
                              pulse.getAmplitude()[i - 1]) -
            t_upcross;
        tot_progress = false;
      }
    }

    // Fill CALOROCSamples and RawCALOROCHit
    auto out_digi_hit = out_digi_hits->create();
    out_digi_hit.setCellID(pulse.getCellID());
    out_digi_hit.setSamplePhase(std::llround(m_cfg.adc_phase / m_cfg.dyRangeTOA * m_cfg.capTOA));
    out_digi_hit.setTimeStamp(time_stamp);

    for (const auto& raw_sample : raw_samples) {
      auto adc =
          std::max(std::llround(raw_sample.adc / m_cfg.dyRangeSingleGainADC * m_cfg.capADC), 0LL);
      auto toa = std::max(std::llround(raw_sample.toa / m_cfg.dyRangeTOA * m_cfg.capTOA), 0LL);
      auto tot = std::max(std::llround(raw_sample.tot / m_cfg.dyRangeTOT * m_cfg.capTOT), 0LL);

      out_digi_hit.addToASamples([&]() {
        edm4eic::CALOROC1ASample aSample;
        aSample.ADC               = adc > m_cfg.capADC ? m_cfg.capADC : adc;
        aSample.timeOfArrival     = toa > m_cfg.capTOA ? m_cfg.capTOA : toa;
        aSample.timeOverThreshold = tot > m_cfg.capTOT ? m_cfg.capTOT : tot;
        return aSample;
      }());

      auto high_adc =
          std::max(std::llround(raw_sample.adc / m_cfg.dyRangeHighGainADC * m_cfg.capADC), 0LL);
      auto low_adc =
          std::max(std::llround(raw_sample.adc / m_cfg.dyRangeLowGainADC * m_cfg.capADC), 0LL);

      out_digi_hit.addToBSamples([&]() {
        edm4eic::CALOROC1BSample bSample;
        bool overflow       = high_adc > m_cfg.capADC;
        bSample.highGainADC = overflow ? m_cfg.capADC : high_adc;
        bSample.lowGainADC =
            overflow ? std::min(low_adc, static_cast<long long>(m_cfg.capADC)) : 0LL;
        bSample.timeOfArrival = toa > m_cfg.capTOA ? m_cfg.capTOA : toa;
        return bSample;
      }());
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
#endif
