// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim
//

#include <cstddef>
#include <gsl/pointers>
#include <random>
#include <vector>

#include "PulseDigi.h"

class HGCROCRawSample {
public:
  HGCROCRawSample(std::size_t n_meas) : meas_types(n_meas, 0), meas_values(n_meas, 0) {}

  void addAmplitude(std::size_t idx, float amp) {
    meas_values[idx] = std::max(meas_values[idx], amp);
  }
  void addUpCrossTime(std::size_t idx, double t) {
    meas_types[idx]  = 1;
    meas_values[idx] = t;
  }
  void addDownCrossTime(std::size_t idx, double t) {
    meas_types[idx]  = 2;
    meas_values[idx] = t;
  }

  std::vector<uint8_t> getMeasTypes() { return meas_types; }
  std::vector<double> getMeasValues() { return meas_values; }

private:
  std::vector<uint8_t> meas_types;
  std::vector<double> meas_values;
};

namespace eicrecon {

void PulseDigi::init() {}

void PulseDigi::process(const PulseDigi::Input& input, const PulseDigi::Output& output) const {
  const auto [in_pulses]                 = input;
  auto [out_digi_hits, out_digi_samples] = output;

  for (const auto& pulse : *in_pulses) {
    float pulse_t            = pulse.getTime();
    float pulse_dt           = pulse.getInterval();
    std::size_t n_amplitudes = pulse.getAmplitude().size();
    ;

    const std::size_t idx_begin = static_cast<std::size_t>(std::floor(pulse_t / m_cfg.time_window));
    const std::size_t idx_end   = static_cast<std::size_t>(
        std::floor((pulse_t + (n_amplitudes - 1) * pulse_dt) / m_cfg.time_window));

    HGCROCRawSample raw_sample(idx_end - idx_begin + 1);

    bool tot_progress = false;
    bool tot_complete = false;

    for (std::size_t i = 0; i < n_amplitudes; i++) {
      double t = pulse_t + i * pulse_dt;
      const std::size_t idx =
          static_cast<std::size_t>(std::floor(t / m_cfg.time_window)) - idx_begin;

      if (!tot_progess) {
        raw_sample.addAmplitude(idx, pulse.getAmplitude()[i]);

        if (pulse.getAmplitude()[i] > m_cfg.threshold) {
          raw_sample.addUpCrossTime(idx, get_crossing_time(m_cfg.threshold, t, pulse_dt,
                                                           pulse.getAmplitude()[i],
                                                           pulse.getAmplitude()[i - 1]));
          tot_progress = true;
          tot_complete = false;
        }
      }

      if (tot_progress && !tot_complete && pulse.getAmplitude()[i] < m_cfg.threshold) {
        pulse_info.addDownCrossTime(idx, get_crossing_time(m_cfg.threshold, t, pulse_dt,
                                                           pulse.getAmplitude()[i],
                                                           pulse.getAmplitude()[i - 1]));
        tot_complete   = true;
        tot_progess = false;
      }
    }
  }
} // PulseDigi:process

double PulseDigi::get_crossing_time(double thres, double t1, double t2, double amp1,
                                    double amp2) const {
  double numerator   = (thres - amp2) * (t2 - t1);
  double denomenator = amp2 - amp1;
  double added       = t2;
  return (numerator / denominator) + added;
}
} // namespace eicrecon
