// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck, Sylvester Joosten, Dmitry Romanov

#pragma once

#include <DD4hep/DD4hepUnits.h>
#include <edm4eic/unit_system.h>

namespace eicrecon {

struct SiliconTrackerDigiConfig {
  // sub-systems should overwrite their own
  // NB: be aware of thresholds in npsim! E.g. https://github.com/eic/npsim/pull/9/files
  double threshold = 0 * dd4hep::keV;
  // Gaussian sigma of the hit time smearing
  double timeResolution = 8 * edm4eic::unit::ns;
  // if > 0, drop cells whose summed energy deposit is below this value
  double cellThreshold = 0 * edm4eic::unit::keV;
  // if > 0, the hit time is the middle of the readout frame it falls in (frame length in ns,
  // frames start at integrationPhase), as for MAPS sensors; timeResolution is then not used
  double integrationTime  = 0 * edm4eic::unit::ns;
  double integrationPhase = 0 * edm4eic::unit::ns;
};

} // namespace eicrecon
