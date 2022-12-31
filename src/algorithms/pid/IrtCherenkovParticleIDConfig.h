// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <Evaluator/DD4hepUnits.h>
#include <spdlog/spdlog.h>

namespace eicrecon {

  // radiator config parameters
  struct RadiatorConfig {
    std::string smearingMode;
    double      smearing;
    double      referenceRIndex;
    double      attenuation;
    int         zbins;
  };

  // IRT algorithm config
  class IrtCherenkovParticleIDConfig {
    public:

      // NOTE: the values hard-coded here are defaults, may be out-of-date, and
      //       should be overridden externally

      unsigned numRIndexBins = 100; // number of bins for refractive index vs. energy

      // radiator-specific settings
      std::map <std::string,RadiatorConfig> radiators = {
        { "Aerogel", RadiatorConfig{
                                     "gaussian",     // smearingMode
                                     2*dd4hep::mrad, // smearing
                                     1.0190,         // referenceRIndex
                                     48*dd4hep::mm,  // attenuation
                                     5,              // zbins
                                   }},
        { "Gas",     RadiatorConfig{
                                     "gaussian",     // smearingMode
                                     5*dd4hep::mrad, // smearing
                                     1.00076,        // referenceRIndex
                                     0*dd4hep::mm,   // attenuation
                                     10,             // zbins
                                   }}
      };

      // list of PDG codes for PID
      std::vector<int> pdgList = {
        -11,
        211,
        321,
        2212
      };

      /////////////////////////////////////////////////////

      // print all parameters
      void Print(std::shared_ptr<spdlog::logger> m_log, spdlog::level::level_enum lvl=spdlog::level::debug) {
        m_log->log(lvl, "{:=^60}"," IrtCherenkovParticleIDConfig Settings ");
        auto puts = [&m_log, &lvl] (auto name, auto val) {
          m_log->log(lvl, "  {:>20} = {:<}", name, val);
        };
        puts("numRIndexBins",numRIndexBins);
        m_log->log(lvl, "pdgList:");
        for(const auto& pdg : pdgList) m_log->log(lvl, "  {}", pdg);
        for(const auto& [name,rad] : radiators) {
          m_log->log(lvl, "{:-<60}", fmt::format("--- {} config ",name));
          puts("smearingMode",    rad.smearingMode);
          puts("smearing",        rad.smearing);
          puts("referenceRIndex", rad.referenceRIndex);
          puts("attenuation",     rad.attenuation);
          puts("zbins",           rad.zbins);
        }
        m_log->log(lvl, "{:=^60}","");
      }

  };
}
