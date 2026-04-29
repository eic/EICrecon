// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson

#pragma once

#include <DD4hep/DD4hepUnits.h>

namespace eicrecon {

struct TrackClusterSubtractorConfig {

  ///! fraction of track energy to subtract
  double energyFractionToSubtract = 1.0;

  ///! default PDG code to use for mass in track
  ///! energy calculation
  int32_t defaultPDG = 211;

  ///! index of surface to use for measuring momentum
  ///! as defined in the CalorimeterTrackProjections
  ///! collection; see the TrackPropagation algorithm
  ///! for implementation details
  uint8_t surfaceToUse = 1;

  ///! turn on/off checking against resolutions
  bool doNSigmaCut = false;

  ///! max no. of sigma to be consistent w/ zero
  uint32_t nSigmaMax = 1;

  ///! calorimeter energy resolution to use
  double calorimeterResolution = 1.0 * dd4hep::GeV;

}; // end TrackClusterSubtractorConfig

} // namespace eicrecon
