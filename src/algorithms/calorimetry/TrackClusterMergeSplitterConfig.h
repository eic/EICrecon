// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Derek Anderson

#pragma once

#include <string>

namespace eicrecon {

struct TrackClusterMergeSplitterConfig {

  std::string idCalo = "HcalBarrel_ID"; // id of calorimeter to match projections to

  double minSigCut = -1.; // min significance
  double avgEP     = 1.0; // mean E/p
  double sigEP     = 1.0; // rms of E/p
  double drAdd     = 0.4; // window to add clusters
  double sampFrac  = 1.0; // allows for sampling fraction correction

  // scale for hit-track distance
  double transverseEnergyProfileScale = 1.0;

}; // end TrackClusterMergeSplitterConfig

} // namespace eicrecon
