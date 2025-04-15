// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Whitney Armstrong, Wouter Deconinck, Sylvester Joosten, Dmitry Romanov, Yann Bedfer

#pragma once

#include <DD4hep/DD4hepUnits.h>

namespace eicrecon {

struct MPGDTrackerDigiConfig {
  // sub-systems should overwrite their own (see "detectors/MPGD/MPGD.cc")

  // Readout identifiers for tagging 1st and 2nd coord. of 2D-strip readout
  std::string readout{""};
  // NB: be aware of thresholds in npsim! E.g. https://github.com/eic/npsim/pull/9/files
  double threshold      = 0 * dd4hep::keV;
  double timeResolution = 8; /// TODO 8 of what units??? Same TODO in juggler. Probably [ns]
};

} // namespace eicrecon
