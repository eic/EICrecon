// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Christopher Dilks

#pragma once

#include <spdlog/spdlog.h>

namespace eicrecon {

  class MergeParticleIDConfig {
    public:

      /////////////////////////////////////////////////////
      // CONFIGURATION PARAMETERS
      //   NOTE: some defaults are hard-coded here; override externally

      // decide how to combine the weights
      enum math_enum { kAddWeights, kMultiplyWeights };
      int mergeMode = kAddWeights;

      //
      /////////////////////////////////////////////////////

      // print all parameters
      void Print(
          std::shared_ptr<spdlog::logger> m_log,
          spdlog::level::level_enum lvl=spdlog::level::debug
          )
      {
        m_log->log(lvl, "{:=^60}"," MergeParticleIDConfig Settings ");
        auto print_param = [&m_log, &lvl] (auto name, auto val) {
          m_log->log(lvl, "  {:>20} = {:<}", name, val);
        };
        print_param("mergeMode",mergeMode);
        m_log->log(lvl, "{:=^60}","");
      }

  };
}
