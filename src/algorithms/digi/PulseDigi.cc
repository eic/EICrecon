// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim
//

#include <cstddef>
#include <gsl/pointers>
#include <random>
#include <vector>

#include "PulseDigi.h"

namespace eicrecon {

void PulseDigi::init() {}

void PulseDigi::process(const PulseDigi::Input& input, const PulseDigi::Output& output) const {
  const auto [in_pulses]                 = input;
  auto [out_digi_hits, out_digi_samples] = output;

  for (const auto& pulse : *in_pulses) {
    bool sample_boundary = false;
    double amplitude_max = -std::numeric_limits<float>::infinity();
    std::vector<float> amplitudes_neighboring(2);
    double toa         = 0;
    double tot         = 0;
    bool totInProgress = false;
    bool totComplete   = false;

    auto out_digi_hit = out_digi_hits->create();

    for (std::size_t i = 0; i < pulse.getAmplitude().size(); i++) {
      double t                      = pulse.getTime() + i * pulse.getInterval();
      sample_boundary               = is_sample_boundary(t, m_cfg.sample_period);
      amplitude                     = pulse.getAmplitude()[i];
      amplitudes_neighboring[i % 2] = amplitude;
      amplitude_max                 = std::max(amplitude_max, amplitude);

      if (!totInProgress && amplitude > m_cfg.threshold) {
        toa           = get_crossing_time(m_cfg.threshold, t, t - pulse.getInterval(),
                                          amplitudes_neiboring[i % 2], amplitudes_neighboring[1 - i % 2]);
        totInProgress = true;
      }

      if (totInProgress && !totComplete && amplitude < m_cfg.threshold) {
        totComplete   = true;
        totInProgress = false;
        tot = get_crossing_time(((m_cfg.threshold - amps_neighbor[i % 2]) * pulse.getInterval()) /
                                (amps_neighbor[i % 2] - amps_neighbor[1 - (i % 2)])) +
              t - toa;
      }

      if (sample_boundary) {
        auto out_digi_sample = out_digi_samples->create();

        out_digi_sample.setADC(amplitude_max);
        out_digi_sample.setTimeOfArrival(toa);
        out_digi_sample.setTimeOverThreshold(tot);
        out_digi_sample.setTOTInProgress(totInProgress);
        out_digi_sample.setTOTComplete(totComplete);
        out_digi_hit.addToSamples(out_digi_hit);

        amplitude_max = -std::numeric_limits<float>::infinity();
        toa           = 0;
        tot           = 0;
      }
    }

    out_digi_hit.setCellID(pulse.getCellID());
    out_digi_hit.setSamplePahse();
    out_digi_hit.setTimeStap();
  }
} // PulseDigi:process

bool PulseDigi::is_sample_boundary(double t, double period) const {
  double n = std::llround(t / period);
  return std::abs(t - n * period) < 1.0e-5;
}

double PulseDigi::get_crossing_time(double thres, double t1, double t2, double amp1,
                                    double amp2) const {
  double numerator   = (thres - amp2) * (t2 - t1);
  double denomenator = amp2 - amp1;
  double added       = t2;
  return (numerator / denominator) + added;
}
} // namespace eicrecon
