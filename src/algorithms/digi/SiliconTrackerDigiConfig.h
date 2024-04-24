// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck, Sylvester Joosten, Dmitry Romanov

#pragma once

#include <DD4hep/DD4hepUnits.h>
#include <edm4eic/unit_system.h>


namespace eicrecon {

  struct SiliconTrackerDigiConfig {
    // sub-systems should overwrite their own
    // NB: be aware of thresholds in npsim! E.g. https://github.com/eic/npsim/pull/9/files
    double threshold  = 0 * dd4hep::keV;
    double timeResolution = 8 * edm4eic::unit::ns; // Source? This is arbitrary --> however, it should be >0, maybe large, to not distort the fitter by default
  };

} // eicrecon
