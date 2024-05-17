// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <Evaluator/DD4hepUnits.h>
#include <spdlog/spdlog.h>

namespace eicrecon {

  // radiator config parameters
  struct RadiatorConfig {
    double      referenceRIndex; // reference radiator refractive index
    double      attenuation;     // reference radiator attenuation length [mm]; set to 0 to disable
    std::string smearingMode;    // smearing type: "gaussian", "uniform"
    double      smearing;        // smearing amount [radians]
  };

  // IRT algorithm config
  class IrtCherenkovParticleIDConfig {
    public:

      /////////////////////////////////////////////////////
      // CONFIGURATION PARAMETERS
      //   NOTE: some defaults are hard-coded here; override externally

      unsigned numRIndexBins = 100; // number of bins to interpolate the refractive index vs. energy

      /* radiator-specific settings; handled by `RadiatorConfig` struct (see above)
       * example: radiators.insert({ "Aerogel", RadiatorConfig{ ... }});
       *          radiators.insert({ "Gas", RadiatorConfig{ ... }});
       */
      std::map <std::string,RadiatorConfig> radiators;

      /* list of PDG codes to identify with this PID algorithm
       * example: std::vector<int> pdgList = { 11, 211, 321, 2212 };
       */
      std::vector<int> pdgList;

      /* cheat modes: useful for test purposes, or idealizing; the real PID should run with all
       * cheat modes off
       */
      bool cheatPhotonVertex  = false; // if true, use MC photon vertex, wavelength, and refractive index
      bool cheatTrueRadiator  = false; // if true, use MC truth to obtain true radiator, for each hit

      //
      /////////////////////////////////////////////////////

      // print warnings about cheat modes
      void PrintCheats(
          std::shared_ptr<spdlog::logger> m_log,
          spdlog::level::level_enum lvl=spdlog::level::debug,
          bool printAll=false
          )
      {
        auto print_param = [&m_log, &lvl, &printAll] (auto name, bool val, auto desc) {
          if(printAll) m_log->log(lvl, "  {:>20} = {:<}", name, val);
          else if(val) m_log->warn("CHEAT MODE '{}' ENABLED: {}", name, desc);
        };
        print_param("cheatPhotonVertex",  cheatPhotonVertex,  "use MC photon vertex, wavelength, refractive index");
        print_param("cheatTrueRadiator",  cheatTrueRadiator,  "use MC truth to obtain true radiator");
      }

      // boolean: true if any cheat mode is enabled
      bool CheatModeEnabled() const {
        return cheatPhotonVertex || cheatTrueRadiator;
      }

      // print all parameters
      void Print(
          std::shared_ptr<spdlog::logger> m_log,
          spdlog::level::level_enum lvl=spdlog::level::debug
          )
      {
        m_log->log(lvl, "{:=^60}"," IrtCherenkovParticleIDConfig Settings ");
        auto print_param = [&m_log, &lvl] (auto name, auto val) {
          m_log->log(lvl, "  {:>20} = {:<}", name, val);
        };
        print_param("numRIndexBins",numRIndexBins);
        PrintCheats(m_log, lvl, true);
        m_log->log(lvl, "pdgList:");
        for(const auto& pdg : pdgList) m_log->log(lvl, "  {}", pdg);
        for(const auto& [name,rad] : radiators) {
          m_log->log(lvl, "{:-<60}", fmt::format("--- {} config ",name));
          print_param("smearingMode",    rad.smearingMode);
          print_param("smearing",        rad.smearing);
          print_param("referenceRIndex", rad.referenceRIndex);
          print_param("attenuation",     rad.attenuation);
        }
        m_log->log(lvl, "{:=^60}","");
      }

  };
}
