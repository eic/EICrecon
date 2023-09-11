// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Dmitry Kalinkin

#pragma once

#include "ChargedParticleSelector.h"

namespace eicrecon {

  void ChargedParticleSelector::init(std::shared_ptr<spdlog::logger>& logger) {
    m_log = logger;
  }

  std::unique_ptr<edm4hep::MCParticleCollection> ChargedParticleSelector::process(const edm4hep::MCParticleCollection &inputs) {
    auto outputs = std::make_unique<edm4hep::MCParticleCollection>();
    output->setSubsetCollection();

    for (cont auto &particle : inputs) {
      if (particle.getCharge() != 0.) {
        outputs.push_back(particle);
      }
    }

    return std::move(outputs)
  }

}
