// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <Evaluator/DD4hepUnits.h>
#include <spdlog/spdlog.h>

namespace eicrecon {

  // radiator config parameters
  struct RadiatorConfig {
    int         id;
    std::string smearingMode;
    double      smearing;
    double      referenceRIndex;
    double      attenuation;
    int         zbins;
  };

  // IRT algorithm config
  class IrtParticleIDConfig {
    public:

      // NOTE: the values hard-coded here are defaults, may be out-of-date, and
      //       should be overridden externally
      unsigned numRIndexBins = 100;
      std::map <std::string,RadiatorConfig> radiators = {
        { "Aerogel", RadiatorConfig{
                                     0,              // id
                                     "gaussian",     // smearingMode
                                     2*dd4hep::mrad, // smearing
                                     1.0190,         // referenceRIndex
                                     48*dd4hep::mm,  // attenuation
                                     5,              // zbins
                                   }},
        { "Gas",     RadiatorConfig{
                                     1,              // id
                                     "gaussian",     // smearingMode
                                     5*dd4hep::mrad, // smearing
                                     1.00076,        // referenceRIndex
                                     0*dd4hep::mm,   // attenuation
                                     10,             // zbins
                                   }}
      };


      // ------------------------------------------------

      // print all parameters
      void Print(std::shared_ptr<spdlog::logger> m_log, spdlog::level::level_enum lvl=spdlog::level::debug) {
        m_log->log(lvl, "{:=^60}"," IrtParticleIDConfig Settings ");
        auto puts = [&m_log, &lvl] (auto name, auto val) {
          m_log->log(lvl, "  {:>20} = {:<}", name, val);
        };
        puts("numRIndexBins",numRIndexBins);
        for(const auto& [name,rad] : radiators) {
          m_log->log(lvl, "{:-<60}", fmt::format("--- {} config ",name));
          puts("id",              rad.id);
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
