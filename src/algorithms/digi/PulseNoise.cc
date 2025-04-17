// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Simon Gardner
//
// Adds noise to a time series pulse
//

#include <podio/RelationRange.h>
#include <gsl/pointers>

#include "PulseNoise.h"

namespace eicrecon {

void PulseNoise::init() {
  m_noise = dd4hep::detail::FalphaNoise(m_cfg.poles, m_cfg.variance, m_cfg.alpha);
}

void PulseNoise::process(const PulseNoise::Input& input, const PulseNoise::Output& output) {
  const auto [inPulses] = input;
  auto [outPulses]      = output;

  for (const auto& pulse : *inPulses) {

    //Clone input pulse to a mutable output pulse
    auto out_pulse = outPulses->create();
    out_pulse.setCellID(pulse.getCellID());
    out_pulse.setInterval(pulse.getInterval());
    out_pulse.setTime(pulse.getTime());

    float integral = 0;
    //Add noise to the pulse
    for (int i = 0; i < pulse.getAmplitude().size(); i++) {
      double noise = m_noise(generator) * m_cfg.scale;
      out_pulse.addToAmplitude(pulse.getAmplitude()[i] + noise);
      integral += pulse.getAmplitude()[i] + noise;
    }

#if EDM4EIC_VERSION_MAJOR >= 8 && EDM4EIC_VERSION_MINOR >= 1
    out_pulse.setIntegral(integral);  
    out_pulse.setPosition(pulse.getPosition());
    for (auto subpulse : pulse.getPulses()) {
      out_pulse.addToPulses(subpulse);
    }
    for (auto particle : pulse.getParticles()) {
      out_pulse.addToParticles(particle);
    }                  
    // Not sure if we want/need to keep the hits themselves at this point?
    for (auto hit : pulse.getTrackerHits()) {
      out_pulse.addToTrackerHits(hit);
    }
    for (auto hit : pulse.getCalorimeterHits()) {
      out_pulse.addToCalorimeterHits(hit);
    }
  
#endif
  }

} // PulseNoise:process
} // namespace eicrecon
