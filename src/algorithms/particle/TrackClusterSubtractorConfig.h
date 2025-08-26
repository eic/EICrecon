// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson

#pragma once

#include <string>

namespace eicrecon {

struct TrackClusterSubtractorConfig {

  // general parameters
  double fracEnergyToSub = 1.0; ///< fraction of track energy to subtract
  int32_t defaultMassPdg = 211; ///< default mass to use for track energy
  uint64_t surfaceToUse  = 1;   ///< index of surface to use for measuring momentum

  // parameters for resolution-based
  // comparison
  bool doNSigmaCut   = false; ///< turn on/off checking against resolutions
  uint32_t nSigmaMax = 1;     ///< max no. of sigma to be consistent w/ zero
  double trkReso     = 1.0;   ///< tracking momentum resolution to use
  double calReso     = 1.0;   ///< calorimeter energy resolution to use

}; // end TrackClusterSubtractorConfig

} // namespace eicrecon
