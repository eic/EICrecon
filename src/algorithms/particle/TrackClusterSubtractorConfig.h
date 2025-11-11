// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson

#pragma once


namespace eicrecon {

struct TrackClusterSubtractorConfig {

  ///! fraction of track energy to subtract
  double fracEnergyToSub = 1.0;

  ///! default mass to use for track energy
  int32_t defaultMassPdg = 211;

  ///! index of surface to use for measuring momentum
  uint64_t surfaceToUse = 1;

  ///! turn on/off checking against resolutions
  bool doNSigmaCut = false;

  ///! max no. of sigma to be consistent w/ zero
  uint32_t nSigmaMax = 1;

  ///! tracking momentum resolution to use
  double trkReso = 1.0;

  ///! calorimeter energy resolution to use
  double calReso = 1.0;

}; // end TrackClusterSubtractorConfig

} // namespace eicrecon
