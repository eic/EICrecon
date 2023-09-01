// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Dmitry Kalinkin

#pragma once

#include <memory>

#include <edm4hep/MCParticleCollection.h>
#include <spdlog/logger.h>

#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

  class ChargedParticleSelector : public WithPodConfig<NoConfig> {

  private:

    std::shared_ptr<spdlog::logger> m_log;

  public:

    void init(std::shared_ptr<spdlog::logger>& logger);
    std::unique_ptr<edm4hep::MCParticleCollection> process(const edm4hep::MCParticleCollection &inputs);

  };

}
