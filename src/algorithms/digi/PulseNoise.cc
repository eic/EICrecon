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

#include "PulseNoise.h"

namespace eicrecon {

void PulseNoise::init() {
    m_poles    = m_cfg.poles;
    m_varience = m_cfg.varience;
    m_alpha    = m_cfg.alpha;
    m_noise    = dd4hep::detail::FalphaNoise(m_poles, m_varience, m_alpha);
}

void PulseNoise::process(const PulseNoise::Input& input,
                         const PulseNoise::Output& output) {
  const auto [inPulses] = input;
  auto [outPulses]     = output;

  for (const auto& pulse : *inPulses) {

    //Clone input pulse to a mutable output pulse
    auto out_pulse = outPulses->create();
    out_pulse.setCellID  (pulse.getCellID());
    out_pulse.setInterval(pulse.getInterval());
    out_pulse.setTime    (pulse.getTime());

    //Add noise to the pulse
    for (int i = 0; i < pulse.getAmplitude().size(); i++) {
      double noise = m_noise(generator);
      out_pulse.addToAmplitude(pulse.getAmplitude()[i] + noise);
    }
    
  }

} // PulseNoise:process
} // namespace eicrecon
