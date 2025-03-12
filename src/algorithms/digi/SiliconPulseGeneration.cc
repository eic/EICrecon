// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul, Chun Yuen Tsang, Prithwish Tribedy
// Special Acknowledgement: Kolja Kauder
//
// Convert energy deposition into ADC pulses
// ADC pulses are assumed to follow the shape of landau function

#include <DDRec/CellIDPositionConverter.h>
#include <Evaluator/DD4hepUnits.h>
#include <cmath>
#include <gsl/pointers>
#include <unordered_map>
#include <vector>

#include "SiliconPulseGeneration.h"

namespace eicrecon {

void SiliconPulseGeneration::init() {
    m_pulse = m_cfg.pulse_shape_function;
}

void SiliconPulseGeneration::process(const SiliconPulseGeneration::Input& input,
                                     const SiliconPulseGeneration::Output& output) const {
  const auto [simhits] = input;
  auto [rawADCs]       = output;

  for (const auto& hit : *simhits) {

    auto   cellID = hit.getCellID();
    double time   = hit.getTime() * dd4hep::ns;
    double charge = hit.getEDep();

    // Calculate nearest timestep to the hit time rounded down (assume clocks aligned with time 0)
    double signal_time = m_cfg.timestep*static_cast<int>(time / m_cfg.timestep);
    
    auto time_series = rawADCs->create();
    time_series.setCellID(cellID);
    time_series.setTime(signal_time);
    time_series.setInterval(m_cfg.timestep);
    
    m_pulse->setHitCharge(charge);
    m_pulse->setHitTime(time);

    for(int i = signal_time; i < m_max_time_bins; i ++) {
      double t = signal_time + i*m_cfg.timestep;
      auto signal = (*m_pulse)(t);
      // std::cout << "Signal: " << signal << std::endl;
      if (signal < m_cfg.ignore_thres) break;
      time_series.addToAmplitude(signal);
    }
  }

} // SiliconPulseGeneration:process
} // namespace eicrecon
