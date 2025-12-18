// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Whitney Armstrong, Wouter Deconinck, Sylvester Joosten, Dmitry Romanov, Yann Bedfer

#pragma once

#include <DD4hep/DD4hepUnits.h>

namespace eicrecon {

struct MPGDTrackerDigiConfig {
  // sub-systems should overwrite their own (see "detectors/MPGD/MPGD.cc")

  // Readout identifiers for tagging 1st and 2nd coord. of 2D-strip readout
  std::string readout{""};

  // - MPGD digitization parameters should come in pairs: one parameter per
  //  strip coordinate.
  // - "MPGDTrackerDigi" is not only digitization but also simulation. Which
  //  latter also relies on parameters. This time, a priori, one and only one
  //  for the whole detector.
  // => So that typically, one gets a set of 1(simulation) + 2(digitization) = 3
  //  parameters in all.
  //   E.g.: The parametrization of EDep -> amplitude of digit would be:
  //      Detector gain (1 simulation parameter)
  //        \times
  //      Charge Sharing \times Electronics amplification (2 digitization parameters, one per coordinate).
  // - Applying the simulation parameter to both coordinates imprints a
  //  correlation between the two.
  // - Here we simplify. With some justification at times: e.g. timing is
  //  dominated by primary electron => To first approximation, one and same
  //  "timeResolution" for both coordinates..

  double gain = 10000;
  std::array<double,2> stripResolutions = {150 * dd4hep::um, 150 * dd4hep::um};
  std::array<int,2>    stripNumbers = {1024, 1024}; // per module
  // NB: be aware of thresholds in npsim! E.g. https://github.com/eic/npsim/pull/9/files
  double threshold      = 0 * dd4hep::keV;
  double timeResolution = 8; // what units???
};

} // namespace eicrecon
