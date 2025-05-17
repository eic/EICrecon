// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Chun Yuen Tsang

#pragma once

#include <edm4eic/unit_system.h>

namespace eicrecon {
struct LGADHitClusterAssociationConfig {
  std::string readout = "TOFBarrelHits";
  double assoDeltaT   = 1 * edm4eic::unit::ns;
};
} // namespace eicrecon
