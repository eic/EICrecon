// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Chun Yuen Tsang

#pragma once
#include <edm4eic/unit_system.h>

namespace eicrecon {
struct LGADHitClusteringConfig {
  std::string readout = "TOFBarrelHits";
  double deltaT       = 1 * edm4eic::unit::ns;
  bool useAve         = false;
};
} // namespace eicrecon
