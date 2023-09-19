// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Dmitry Kalinkin

#pragma once

#include <memory>

namespace edm4hep { class MCParticleCollection; }
namespace spdlog { class logger; }

namespace eicrecon {

  class ChargedParticleSelector {

  private:

    std::shared_ptr<spdlog::logger> m_log;

  public:

    void init(std::shared_ptr<spdlog::logger>& logger);
    std::unique_ptr<edm4hep::MCParticleCollection> process(const edm4hep::MCParticleCollection &inputs);

  };

}
