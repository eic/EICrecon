// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <spdlog/spdlog.h>

namespace eicrecon {

  class ParticleIDConfig {
    public:

      /////////////////////////////////////////////////////
      // CONFIGURATION PARAMETERS
      //   NOTE: some defaults are hard-coded here; override externally

      bool highestWeightOnly = true; // if true, write out only the ParticleID with the highest weight

      //
      /////////////////////////////////////////////////////

      // print all parameters
      void Print(
          std::shared_ptr<spdlog::logger> m_log,
          spdlog::level::level_enum lvl=spdlog::level::debug
          )
      {
        m_log->log(lvl, "{:=^60}"," ParticleIDConfig Settings ");
        auto puts = [&m_log, &lvl] (auto name, auto val) {
          m_log->log(lvl, "  {:>20} = {:<}", name, val);
        };
        puts("highestWeightOnly",highestWeightOnly);
        m_log->log(lvl, "{:=^60}","");
      }

  };
}
