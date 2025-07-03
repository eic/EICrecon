// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Simon Gardner
//
// Adds noise to a time series pulse
//

#include <DDDigi/noise/FalphaNoise.h>
#include <edm4hep/MCParticle.h>
#include <edm4hep/SimCalorimeterHit.h>
#include <edm4hep/SimTrackerHit.h>
#include <podio/RelationRange.h>
#include <cstddef>
#include <gsl/pointers>
#include <random>
#include <vector>

#include "PulseNoise.h"

namespace eicrecon {

void PulseNoise::init() {}

void PulseNoise::process(const PulseNoise::Input& input, const PulseNoise::Output& output) const {
  const auto [headers, inPulses] = input;
  auto [outPulses]               = output;

  // local random generator
  auto seed = m_uid.getUniqueID(*headers, name());
  std::default_random_engine generator(seed);
  dd4hep::detail::FalphaNoise falpha(m_cfg.poles, m_cfg.variance, m_cfg.alpha);

  for (const auto& pulse : *inPulses) {

    //Clone input pulse to a mutable output pulse
    auto out_pulse = outPulses->create();
    out_pulse.setCellID(pulse.getCellID());
    out_pulse.setInterval(pulse.getInterval());
    out_pulse.setTime(pulse.getTime());

    float integral = 0;
    //Add noise to the pulse
    for (std::size_t i = 0; i < pulse.getAmplitude().size(); i++) {
      double noise     = falpha(generator) * m_cfg.scale;
      double amplitude = pulse.getAmplitude()[i] + noise;
      out_pulse.addToAmplitude(amplitude);
      integral += amplitude;
    }

#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 1)
    out_pulse.setIntegral(integral);
    out_pulse.setPosition(pulse.getPosition());
    out_pulse.addToPulses(pulse);

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
