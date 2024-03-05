// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Christopher Dilks

#pragma once

#include <spdlog/spdlog.h>

namespace eicrecon {

  class RichTrackConfig {
    public:

      /////////////////////////////////////////////////////
      // CONFIGURATION PARAMETERS
      //   NOTE: some defaults are hard-coded here; override externally

      std::map <std::string,unsigned> numPlanes; // number of xy-planes for track projections (for each radiator)

      //
      /////////////////////////////////////////////////////


      // print all parameters
      void Print(
          std::shared_ptr<spdlog::logger> m_log,
          spdlog::level::level_enum lvl=spdlog::level::debug
          )
      {
        m_log->log(lvl, "{:=^60}"," RichTrackConfig Settings ");
        for(const auto& [rad,val] : numPlanes)
          m_log->log(lvl, "  {:>20} = {:<}", fmt::format("{} numPlanes", rad), val);
        m_log->log(lvl, "{:=^60}","");
      }

  };
}
