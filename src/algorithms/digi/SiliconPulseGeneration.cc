// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul, Chun Yuen Tsang, Prithwish Tribedy
// Special Acknowledgement: Kolja Kauder
//
// Convert energy deposition into ADC pulses
// ADC pulses are assumed to follow the shape of landau function

#include <Evaluator/DD4hepUnits.h>

#include "SiliconPulseGeneration.h"

namespace eicrecon {

void SiliconPulseGeneration::init() {
    m_pulse = m_cfg.pulse_shape_function;
}

void SiliconPulseGeneration::process(const SiliconPulseGeneration::Input& input,
                                     const SiliconPulseGeneration::Output& output) const {
  const auto [simhits] = input;
  auto [rawPulses]     = output;

  for (const auto& hit : *simhits) {

    auto   cellID = hit.getCellID();
    double time   = hit.getTime() * dd4hep::ns;
    double charge = hit.getEDep();

    // Calculate nearest timestep to the hit time rounded down (assume clocks aligned with time 0)
    double signal_time = m_cfg.timestep*static_cast<int>(time / m_cfg.timestep);

    auto time_series = rawPulses->create();
    time_series.setCellID(cellID);
    time_series.setInterval(m_cfg.timestep);

    m_pulse->setHitCharge(charge);
    m_pulse->setHitTime(time);

    float maxSignalTime = m_pulse->getMaximumTime();

    for(int i = 0; i < m_max_time_bins; i ++) {
      double t = signal_time + i*m_cfg.timestep;
      auto signal = (*m_pulse)(t);
      if (signal < m_cfg.ignore_thres) {
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
