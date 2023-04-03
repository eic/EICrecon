// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <spdlog/spdlog.h>

namespace eicrecon {

  class LinkParticleIDConfig {
    public:

      /////////////////////////////////////////////////////
      // CONFIGURATION PARAMETERS
      //   NOTE: some defaults are hard-coded here; override externally

      // NOTE: cf. ParticlesWithTruthPIDConfig settings
      double momentumRelativeTolerance = 100.0; // Matching momentum effectively disabled
      double phiTolerance              = 0.1;   // Matching phi tolerance [rad]
      double etaTolerance              = 0.2;   // Matching eta tolerance

      //
      /////////////////////////////////////////////////////

      // print all parameters
      void Print(
          std::shared_ptr<spdlog::logger> m_log,
          spdlog::level::level_enum lvl=spdlog::level::debug
          )
      {
        m_log->log(lvl, "{:=^60}"," LinkParticleIDConfig Settings ");
        auto puts = [&m_log, &lvl] (auto name, auto val) {
          m_log->log(lvl, "  {:>20} = {:<}", name, val);
        };
        puts("momentumRelativeTolerance",momentumRelativeTolerance);
        puts("phiTolerance",phiTolerance);
        puts("etaTolerance",etaTolerance);
        m_log->log(lvl, "{:=^60}","");
      }

  };
}
