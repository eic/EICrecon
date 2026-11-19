// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#pragma once

#include <edm4eic/unit_system.h>

namespace eicrecon {

struct CalorimeterHitToTrackerHitConfig {

  // Time uncertainty (sigma_t) assigned to the output tracker hits, in edm4eic units.
  // A value <= 0 (default) propagates the time error of the input calorimeter hit.
  double timeError{0.0 * edm4eic::unit::ns};
};

} // namespace eicrecon
