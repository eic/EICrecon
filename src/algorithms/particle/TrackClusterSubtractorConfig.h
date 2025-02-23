// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson

#pragma once

#include <string>

namespace eicrecon {

  struct TrackClusterSubtractorConfig {

    double   fracEnergyToSub = 1.0;  ///< fraction of track energy to subtract
    int32_t  defaultMassPdg  = 211;  ///< default mass to use for track energy
    uint64_t surfaceToUse    = 1;    ///< index of surface to use for measuring momentum

  };  // end TrackClusterSubtractorConfig

}  // end eicrecon namespace
