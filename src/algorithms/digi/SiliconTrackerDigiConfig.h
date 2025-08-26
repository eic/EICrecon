// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck, Sylvester Joosten, Dmitry Romanov

#pragma once

#include <DD4hep/DD4hepUnits.h>

namespace eicrecon {

struct SiliconTrackerDigiConfig {
  // sub-systems should overwrite their own
  // NB: be aware of thresholds in npsim! E.g. https://github.com/eic/npsim/pull/9/files
  double threshold      = 0 * dd4hep::keV;
  double timeResolution = 8; /// TODO 8 of what units??? Same TODO in juggler. Probably [ns]
};

} // namespace eicrecon
