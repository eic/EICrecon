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
  HGCROCRawSample(std::size_t n_meas)
      : meas_types(n_meas, 0), amplitudes(n_meas, 0), TOAs(n_meas, 0), TOTs(n_meas, 0) {}

  void addAmplitude(std::size_t idx, double amp) { amplitudes[idx] = amp; }
  void addTOA(std::size_t idx, double toa) {
    meas_types[idx] = 1;
    TOAs[idx]       = toa;
  }
  void addTOT(std::size_t idx, double tot) {
    meas_types[idx] = 2;
    TOTs[idx]       = tot;
  }

  const std::vector<uint8_t>& getMeasTypes() const { return meas_types; }
  const std::vector<double>& getAmplitudes() const { return amplitudes; }
  const std::vector<double>& getTOAs() const { return TOAs; }
  const std::vector<double>& getTOTs() const { return TOTs; }

private:
  // meas_type 1: Pulse crossed below the threshold.
  // meas_type 2: Pulse didn't cross the threshold.
  // meas_type 0: Neither of the above cases.
  std::vector<uint8_t> meas_types;
  std::vector<double> amplitudes;
  std::vector<double> TOAs;
  std::vector<double> TOTs;
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

    int sample_tick = std::llround(m_cfg.sample_period / pulse_dt);
    int adc_counter =
        std::llround((timeIdx_begin * m_cfg.sample_period + m_cfg.adc_phase - pulse_t) / pulse_dt);

    bool tot_progress  = false;
    bool tot_complete  = false;
    std::size_t toaIdx = 0;

    for (std::size_t i = 0; i < n_amps; i++) {
      double t = pulse_t + i * pulse_dt;
      const std::size_t sampleIdx =
          static_cast<std::size_t>(std::floor(t / m_cfg.sample_period)) - timeIdx_begin;

      adc_counter++;
      // Measure amplitudes
      if (adc_counter == sample_tick) {
        raw_sample.addAmplitude(sampleIdx, pulse.getAmplitude()[i]);
        adc_counter = 0;
      }

      // Measure crossing point for TOA
      if (!tot_progress && pulse.getAmplitude()[i] > m_cfg.toa_thres) {
        toaIdx = sampleIdx;
        raw_sample.addTOA(sampleIdx,
                          get_crossing_time(m_cfg.threshold, t, pulse_dt, pulse.getAmplitude()[i],
                                            pulse.getAmplitude()[i - 1]));
        tot_progress = true;
        tot_complete = false;
      }

      // Measure crossing point for TOT
      if (tot_progress && !tot_complete && pulse.getAmplitude()[i] < m_cfg.threshold) {
        raw_sample.addTOT(toaIdx,
                          get_crossing_time(m_cfg.threshold, t, pulse_dt, pulse.getAmplitude()[i],
                                            pulse.getAmplitude()[i - 1]));
        tot_complete = true;
        tot_progess  = false;
      }
    }
  }
} // PulseDigi:process

double PulseDigi::get_crossing_time(double thres, double t1, double t2, double amp1,
                                    double amp2) const {
  double numerator   = (thres - amp2) * (t2 - t1);
  double denominator = amp2 - amp1;
  double added       = t2;
  return (numerator / denominator) + added;
}
} // namespace eicrecon
