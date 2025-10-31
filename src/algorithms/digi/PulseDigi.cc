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

#include "PulseDigi.h"

class HGCROCRawSample {
public:
  struct RawEntry {
    double adc{0};
    double toa{0};
    double tot{0};
    bool totProgress{false};
    bool totComplete{false};
  };

  HGCROCRawSample(std::size_t n_samp) { rawEntries.resize(n_samp); }

  void setAmplitude(std::size_t idx, double amp) { rawEntries[idx].adc = amp; }
  void setTOA(std::size_t idx, double toa) { rawEntries[idx].toa = toa; }
  void setTOT(std::size_t idx, double cross_t) {
    rawEntries[idx].tot = cross_t - rawEntries[idx].toa;
  }
  void setTotProgress(std::size_t idx) { rawEntries[idx].totProgress = true; }
  void setTotComplete(std::size_t idx) { rawEntries[idx].totComplete = true; }

  const std::vector<RawEntry>& getEntries() const { return rawEntries; }

private:
  std::vector<RawEntry> rawEntries;
};

namespace eicrecon {

void PulseDigi::init() {}

void PulseDigi::process(const PulseDigi::Input& input, const PulseDigi::Output& output) const {
  const auto [in_pulses] = input;
  auto [out_digi_hits]   = output;

  for (const auto& pulse : *in_pulses) {
    double pulse_t     = pulse.getTime();
    double pulse_dt    = pulse.getInterval();
    std::size_t n_amps = pulse.getAmplitude().size();

    // Estimate the number of samples.
    const std::size_t timeIdx_begin =
        static_cast<std::size_t>(std::floor(pulse_t / m_cfg.time_window));
    const std::size_t timeIdx_end = static_cast<std::size_t>(
        std::floor((pulse_t + (n_amps - 1) * pulse_dt) / m_cfg.time_window));

    HGCROCRawSample raw_sample(timeIdx_end - timeIdx_begin + 1);

    // For ADC, amplitude is measured with a fixed phase.
    // This was reproduced by sample_tick and adc_counter as follows.
    // Amplitude is measured whenever adc_counter reaches sample_tick.
    int sample_tick = std::llround(m_cfg.time_window / pulse_dt);
    int adc_counter =
        std::llround((timeIdx_begin * m_cfg.time_window + m_cfg.adc_phase - pulse_t) / pulse_dt);

    bool tot_progress  = false;
    bool tot_complete  = false;
    std::size_t toaIdx = 0;

    for (std::size_t i = 0; i < n_amps; i++) {
      double t = pulse_t + i * pulse_dt;
      const std::size_t sampleIdx =
          static_cast<std::size_t>(std::floor(t / m_cfg.time_window)) - timeIdx_begin;

      adc_counter++;

      // Measure amplitudes for ADC
      if (adc_counter == sample_tick) {
        raw_sample.setAmplitude(sampleIdx, pulse.getAmplitude()[i]);
        adc_counter = 0;
        if (tot_progress)
          raw_sample.setTotProgress(sampleIdx);
      }

      // Measure up-crossing time for TOA
      if (!tot_progress && pulse.getAmplitude()[i] > m_cfg.toa_thres) {
        toaIdx = sampleIdx;
        raw_sample.setTOA(sampleIdx,
                          get_crossing_time(m_cfg.toa_thres, pulse_dt, t, pulse.getAmplitude()[i],
                                            pulse.getAmplitude()[i - 1]));
        tot_progress = true;
        tot_complete = false;
        raw_sample.setTotProgress(sampleIdx);
      }

      // Measure down-crossing time for TOT
      if (tot_progress && !tot_complete && pulse.getAmplitude()[i] < m_cfg.tot_thres) {
        raw_sample.setTOT(toaIdx,
                          get_crossing_time(m_cfg.tot_thres, pulse_dt, t, pulse.getAmplitude()[i],
                                            pulse.getAmplitude()[i - 1]));
        tot_progress = false;
        tot_complete = true;
        raw_sample.setTotComplete(sampleIdx);
      }
    }

    // Fill HGCROCSamples and RawHGCROCHit
    auto out_digi_hit = out_digi_hits->create();
    out_digi_hit.setCellID(pulse.getCellID());

    const auto& entries = raw_sample.getEntries();

    for (const auto& entry : entries) {
      edm4eic::HGCROCSample sample;
      auto adc   = std::max(std::llround(entry.adc / m_cfg.dyRangeADC * m_cfg.capADC), 0LL);
      sample.ADC = adc > m_cfg.capADC ? m_cfg.capADC : adc;
      auto toa   = std::max(std::llround(entry.toa / m_cfg.dyRangeTOA * m_cfg.capTOA), 0LL);
      sample.timeOfArrival = toa > m_cfg.capTOA ? m_cfg.capTOA : toa;
      auto tot = std::max(std::llround(entry.tot / m_cfg.dyRangeTOT * m_cfg.capTOT), 0LL);
      sample.timeOverThreshold = tot > m_cfg.capTOT ? m_cfg.capTOT : tot;
      sample.TOTInProgress     = entry.totProgress;
      sample.TOTComplete       = entry.totComplete;
      out_digi_hit.addToSamples(sample);
    }
  }
} // PulseDigi:process

double PulseDigi::get_crossing_time(double thres, double dt, double t, double amp1,
                                    double amp2) const {
  double numerator   = (thres - amp2) * dt;
  double denominator = amp2 - amp1;
  double added       = t;
  return (numerator / denominator) + added;
}
} // namespace eicrecon
