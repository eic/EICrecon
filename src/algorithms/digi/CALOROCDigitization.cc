// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim, Wouter Deconinck, Dmitry Kalinkin, Derek Anderson, Simon Gardner, Sylvester Joosten, Maria Zurek
//

#include <edm4eic/CALOROC1ASample.h>
#include <edm4eic/CALOROC1BSample.h>
#include <podio/RelationRange.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <tuple>
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
    const auto& amps   = pulse.getAmplitude();
    std::size_t n_amps = amps.size();

    // Find the first amplitude index above toa_thres.
    // Start from i = 1 since amps[idx_upcross] is used to calculate the crossing time.
    // Pulses that never cross toa_thres are skipped.
    std::size_t idx_upcross = 0;
    for (std::size_t i = 1; i < n_amps; i++) {
      if (amps[i] > m_cfg.toa_thres) {
        idx_upcross = i;
        break;
      }
    }
    if (idx_upcross == 0)
      continue;

    // Interpolate the first up-crossing time so that ADC measurement
    // starts only after it.
    double t_upcross =
        get_crossing_time(m_cfg.toa_thres, pulse_dt, pulse_t + idx_upcross * pulse_dt,
                          amps[idx_upcross], amps[idx_upcross - 1]);

    // Sample index of the first CALOROC measurement after t_upcross
    std::size_t time_stamp =
        static_cast<std::size_t>(std::ceil((t_upcross - m_cfg.adc_phase) / m_cfg.time_window));
    // Amplitude index corresponding to the sampling point given by time_stamp
    std::size_t idx_amp_first = static_cast<std::size_t>(
        (m_cfg.adc_phase + time_stamp * m_cfg.time_window - pulse_t) / pulse_dt);
    // Number of amplitude bins spanned by one time_window
    std::size_t sample_tick = static_cast<std::size_t>(m_cfg.time_window / pulse_dt);

    std::vector<RawEntry> raw_samples(m_cfg.n_samples);

    // ADCs are filled in advance because the measurement indices
    // are already determined.
    // CALOROC measures pulse amplitude for ADC.
    for (std::size_t i = 0; i < m_cfg.n_samples; i++) {
      std::size_t idx_amp = idx_amp_first + i * sample_tick;
      if (idx_amp < n_amps)
        raw_samples[i].adc = amps[idx_amp];
      else
        break;
    }

    std::size_t idx_sample  = 0;
    std::size_t idx_toa     = 0;
    bool is_above_toa_thres = false;
    bool is_above_tot_thres = false;

    // Measure the TOAs and TOTs while scanning the amplitudes.
    // Start from i = 1 since amps[i-1] is used to calculate the crossing time.
    for (std::size_t i = 1; i < n_amps; i++) {
      double t = pulse_t + i * pulse_dt;
      if (i > idx_amp_first)
        idx_sample = (i + sample_tick - idx_amp_first - 1) / sample_tick;
      if (idx_sample == m_cfg.n_samples)
        break;

      // Measure up-crossing time for TOA
      if (!is_above_toa_thres && amps[i] > m_cfg.toa_thres) {
        idx_toa = idx_sample;
        // Recompute t_upcross at every up-crossing, since a single pulse can have
        // more than one TOA.
        t_upcross = get_crossing_time(m_cfg.toa_thres, pulse_dt, t, amps[i], amps[i - 1]);
        raw_samples[idx_toa].toa =
            m_cfg.adc_phase + (time_stamp + idx_toa) * m_cfg.time_window - t_upcross;
        is_above_toa_thres = true;
      }

      if (amps[i] > m_cfg.tot_thres)
        is_above_tot_thres = true;

      // Measure down-crossing time for TOT
      if (is_above_tot_thres && amps[i] < m_cfg.tot_thres) {
        raw_samples[idx_toa].tot =
            get_crossing_time(m_cfg.tot_thres, pulse_dt, t, amps[i], amps[i - 1]) - t_upcross;
        is_above_tot_thres = false;
      }

      if (is_above_toa_thres && amps[i] < m_cfg.toa_thres)
        is_above_toa_thres = false;
    }

    // Fill CALOROCSamples and RawCALOROCHit
    auto out_digi_hit = out_digi_hits->create();
    out_digi_hit.setCellID(pulse.getCellID());
    out_digi_hit.setSamplePhase(std::llround(m_cfg.adc_phase / m_cfg.dyRangeTOA * m_cfg.capTOA));
    out_digi_hit.setTimeStamp(time_stamp);

    for (const auto& raw_sample : raw_samples) {
      auto adc =
          std::clamp(std::llround(raw_sample.adc / m_cfg.dyRangeSingleGainADC * m_cfg.capADC), 0LL,
                     static_cast<long long>(m_cfg.capADC) - 1);
      auto toa = std::clamp(std::llround(raw_sample.toa / m_cfg.dyRangeTOA * m_cfg.capTOA), 0LL,
                            static_cast<long long>(m_cfg.capTOA) - 1);
      auto tot = std::clamp(std::llround(raw_sample.tot / m_cfg.dyRangeTOT * m_cfg.capTOT), 0LL,
                            static_cast<long long>(m_cfg.capTOT) - 1);

      out_digi_hit.addToASamples([&]() {
        edm4eic::CALOROC1ASample aSample;
        aSample.ADC               = adc;
        aSample.timeOfArrival     = toa;
        aSample.timeOverThreshold = tot;
        return aSample;
      }());

      auto high_adc =
          std::clamp(std::llround(raw_sample.adc / m_cfg.dyRangeHighGainADC * m_cfg.capADC), 0LL,
                     static_cast<long long>(m_cfg.capADC) - 1);
      auto low_adc =
          std::clamp(std::llround(raw_sample.adc / m_cfg.dyRangeLowGainADC * m_cfg.capADC), 0LL,
                     static_cast<long long>(m_cfg.capADC) - 1);

      out_digi_hit.addToBSamples([&]() {
        edm4eic::CALOROC1BSample bSample;
        bSample.highGainADC   = high_adc;
        bSample.lowGainADC    = low_adc;
        bSample.timeOfArrival = toa;
        return bSample;
      }());
    }
  }
} // CALOROCDigitization:process

double CALOROCDigitization::get_crossing_time(double thres, double dt, double t, double amp1,
                                              double amp2) const {
  double numerator   = (amp1 - thres) * dt;
  double denominator = amp2 - amp1;
  double added       = t;
  return (numerator / denominator) + added;
}
} // namespace eicrecon
