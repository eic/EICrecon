// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson

#pragma once

#include <string>

namespace eicrecon {

  struct TrackClusterSubtractorConfig {

    uint64_t surfaceToUse = 1;  // index of surface to use for projections
    double fracEnergyToSub = 1.0;  // fraction of energy to subtract

  };  // end TrackClusterSubtractorConfig

}  // end eicrecon namespace
