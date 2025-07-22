// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025, Simon Gardner

#pragma once

#include <vector>
#include <DD4hep/DD4hepUnits.h>

namespace eicrecon {

struct PhaseSpacePIDConfig {
  std::string system;
  std::vector<float> direction = {0.0, 0.0, -1.0}; // default direction is along z-axis
  double opening_angle         = 12 * dd4hep::mrad;
  int pdg_value                = 11; // default as electron
};

} // namespace eicrecon
