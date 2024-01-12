// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Dmitry Kalinkin

#pragma once

#include <memory>
#include <utility>

#include <edm4hep/MCParticleCollection.h>
#include <spdlog/logger.h>

#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

  class ChargedMCParticleSelector : public WithPodConfig<NoConfig> {

  private:

    std::shared_ptr<spdlog::logger> m_log;

  public:

    // algorithm initialization
    void init(std::shared_ptr<spdlog::logger>& logger) {
      m_log = logger;
    }

    // primary algorithm call
    std::unique_ptr<edm4hep::MCParticleCollection> process(const edm4hep::MCParticleCollection* inputs) {
      auto outputs = std::make_unique<edm4hep::MCParticleCollection>();
      outputs->setSubsetCollection();

      for (const auto &particle : *inputs) {
        if (particle.getCharge() != 0.) {
          outputs->push_back(particle);
        }
      }

      return std::move(outputs);
    }

  };

}

