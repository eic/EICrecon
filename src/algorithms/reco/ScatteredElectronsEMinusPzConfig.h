// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Daniel Brandenburg

#pragma once

#include <string>
#include <DD4hep/DD4hepUnits.h>

namespace eicrecon {

struct ScatteredElectronsEMinusPzConfig {

  // For now these are wide open
  // In the future the cut should depend
  // on the generator settings
  float minEMinusPz = 0.0;       // GeV
  float maxEMinusPz = 1000000.0; // GeV
};

} // namespace eicrecon
