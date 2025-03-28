// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Simon Gardner
//
// Convert energy deposition into ADC pulses
// ADC pulses are assumed to follow the shape of landau function

#include "SiliconPulseGeneration.h"

namespace eicrecon {

void SiliconPulseGeneration::init() {
    m_pulse             = PulseShapeFactory::createPulseShape(m_cfg.pulse_shape_function, m_cfg.pulse_shape_params);
    m_ignore_thres      = m_cfg.ignore_thres;
    m_timestep          = m_cfg.timestep;
    m_min_sampling_time = m_cfg.min_sampling_time;
    m_max_time_bins     = m_cfg.max_time_bins;

    if(m_pulse->getMaximumTime()>m_min_sampling_time) {
      m_min_sampling_time = m_pulse->getMaximumTime();
    }
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

    bool passed_threshold = false;

    for(int i = 0; i < m_max_time_bins; i ++) {
      double t = signal_time + i*m_timestep - time;
      auto signal = (*m_pulse)(t,charge);
      if (std::abs(signal) < m_ignore_thres) {
        if(passed_threshold==false) {
          signal_time = t;
          continue;
        }        
        if (t > m_min_sampling_time) {
          break;
        } 
      }
      passed_threshold = true;
      time_series.addToAmplitude(signal);
    }

    time_series.setTime(signal_time);

  }

} // SiliconPulseGeneration:process
} // namespace eicrecon
