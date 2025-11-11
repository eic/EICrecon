// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson, Dmitry Kalinkin

#pragma once

namespace eicrecon {

struct TrackClusterMergeSplitterConfig {

  ///! any clusters below this will be merged
  double minSigCut = -1.0;

  ///! average of E/p distribution
  double avgEP = 1.0;

  ///! RMS of E/p distribution
  double sigEP = 1.0;

  ///! window to merge clusters
  double drAdd = 0.4;

  ///! index of surface to use for track projections
  uint64_t surfaceToUse = 1;

  ///! scale for hit-track distance
  double transverseEnergyProfileScale = 1.0;

}; // end TrackClusterMergeSplitterConfig

} // namespace eicrecon
