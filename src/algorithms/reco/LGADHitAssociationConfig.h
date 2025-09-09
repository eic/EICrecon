// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Chun Yuen Tsang

#pragma once

#include <edm4eic/unit_system.h>
#include <vector>
#include <string>

namespace eicrecon {
struct LGADHitAssociationConfig {
  std::string readout                     = "TOFBarrelHits";
  double assoDeltaT                       = 1 * edm4eic::unit::ns;
  std::vector<std::string> subsensor_keys = {"x", "y"};
};
} // namespace eicrecon
