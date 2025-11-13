// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Xin Dong

#pragma once

#include <string>
#include <edm4eic/unit_system.h>
#include <DD4hep/DD4hepUnits.h>

namespace eicrecon {

struct SecondaryVerticesHelixConfig {

  bool unlikesign   = true;
  float minDca      = 0.03 * edm4eic::unit::mm; // mm, daughter to pVtx
  float maxDca12    = 1. * edm4eic::unit::mm;   // mm, dca between daughter 1 and 2
  float maxDca      = 1. * edm4eic::unit::mm;   // mm, dca of V0 to pVtx
  float minCostheta = 0.8; // costheta, theta: angle of V0 decay direction and momentum
};

} // namespace eicrecon
