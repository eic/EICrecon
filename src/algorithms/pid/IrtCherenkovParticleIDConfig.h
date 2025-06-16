// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <Evaluator/DD4hepUnits.h>
#include <algorithms/logger.h>
#include <spdlog/spdlog.h>

namespace eicrecon {

// radiator config parameters
struct RadiatorConfig {
  double referenceRIndex;   // reference radiator refractive index
  double attenuation;       // reference radiator attenuation length [mm]; set to 0 to disable
  std::string smearingMode; // smearing type: "gaussian", "uniform"
  double smearing;          // smearing amount [radians]
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
  std::map<std::string, RadiatorConfig> radiators;

  /* list of PDG codes to identify with this PID algorithm
       * example: std::vector<int> pdgList = { 11, 211, 321, 2212 };
       */
  std::vector<int> pdgList;

  /* cheat modes: useful for test purposes, or idealizing; the real PID should run with all
       * cheat modes off
       */
  bool cheatPhotonVertex = false; // if true, use MC photon vertex, wavelength, and refractive index
  bool cheatTrueRadiator = false; // if true, use MC truth to obtain true radiator, for each hit

  //
  /////////////////////////////////////////////////////

  // boolean: true if any cheat mode is enabled
  bool CheatModeEnabled() const { return cheatPhotonVertex || cheatTrueRadiator; }

  // stream all parameters
  friend std::ostream& operator<<(std::ostream& os, const IrtCherenkovParticleIDConfig& cfg) {
    os << fmt::format("{:=^60}", " IrtCherenkovParticleIDConfig Settings ") << std::endl;
    auto print_param = [&os](auto name, auto val) {
      os << fmt::format("  {:>20} = {:<}", name, val) << std::endl;
    };
    print_param("numRIndexBins", cfg.numRIndexBins);
    print_param("cheatPhotonVertex", cfg.cheatPhotonVertex);
    print_param("cheatTrueRadiator", cfg.cheatTrueRadiator);
    os << "pdgList:" << std::endl;
    for (const auto& pdg : cfg.pdgList)
      os << fmt::format("  {}", pdg) << std::endl;
    for (const auto& [name, rad] : cfg.radiators) {
      os << fmt::format("{:-<60}", fmt::format("--- {} config ", name)) << std::endl;
      print_param("smearingMode", rad.smearingMode);
      print_param("smearing", rad.smearing);
      print_param("referenceRIndex", rad.referenceRIndex);
      print_param("attenuation", rad.attenuation);
    }
    os << fmt::format("{:=^60}", "") << std::endl;
    return os;
  };
};

} // namespace eicrecon
