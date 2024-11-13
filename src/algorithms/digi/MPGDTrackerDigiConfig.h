// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck, Sylvester Joosten, Dmitry Romanov

#pragma once

#include <DD4hep/DD4hepUnits.h>

namespace eicrecon {

  struct MPGDTrackerDigiConfig {
    // sub-systems should overwrite their own
    // Readout identifiers for dividing detector
    std::string readout{""};
    std::string x_field{"x"};
    std::string y_field{"y"};
    // NB: be aware of thresholds in npsim! E.g. https://github.com/eic/npsim/pull/9/files
    double threshold  = 0 * dd4hep::keV;
    double timeResolution = 8;   /// TODO 8 of what units??? Same TODO in juggler. Probably [ns]
  };

} // eicrecon
