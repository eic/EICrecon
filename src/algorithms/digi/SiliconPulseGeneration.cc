// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Simon Gardner
//
// Convert energy deposition into ADC pulses
// ADC pulses are assumed to follow the shape of landau function

#include "SiliconPulseGeneration.h"

namespace eicrecon {

void SiliconPulseGeneration::init() {
    m_pulse         = m_cfg.pulse_shape_function;
    m_ignore_thres  = m_cfg.ignore_thres;
    m_timestep      = m_cfg.timestep;
    m_max_time_bins = m_cfg.max_time_bins;
}

void SiliconPulseGeneration::process(const SiliconPulseGeneration::Input& input,
                                     const SiliconPulseGeneration::Output& output) const {
  const auto [simhits] = input;
  auto [rawPulses]     = output;

  for (const auto& hit : *simhits) {

    auto   cellID = hit.getCellID();
    double time   = hit.getTime();
    double charge = hit.getEDep();

    // Calculate nearest timestep to the hit time rounded down (assume clocks aligned with time 0)
    double signal_time = m_timestep*std::floor(time / m_timestep);

    auto time_series = rawPulses->create();
    time_series.setCellID(cellID);
    time_series.setInterval(m_timestep);

    m_pulse->setHitCharge(charge);
    m_pulse->setHitTime(time);

    float maxSignalTime = m_pulse->getMaximumTime();

    for(int i = 0; i < m_max_time_bins; i ++) {
      double t = signal_time + i*m_timestep;
      auto signal = (*m_pulse)(t);
      if (signal < m_ignore_thres) {
        if (t > maxSignalTime) {
          break;
        } else {
          signal_time = t;
          continue;
        }
      }
      time_series.addToAmplitude(signal);
    }

    time_series.setTime(signal_time);
  }

} // SiliconPulseGeneration:process
} // namespace eicrecon
