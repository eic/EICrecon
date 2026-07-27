// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Sylvester Joosten, Whitney Armstrong

#pragma once

namespace eicrecon {

struct ImagingClusterRecoConfig {

  int trackStopLayer = 9;

  //! if true, override cluster position using the top N highest-energy hits
  //! within layers <= maxLayersForPos, averaged in eta-phi space
  bool usePositionOfHighestEnergyHit = false;

  //! max layer number included in position estimation
  int maxLayersForPos = 6;

  //! number of hits to average for position; if <= 0, use truncateFrac instead
  int numHitsForPos = -1;

  //! fraction of hits (within maxLayersForPos) to use when numHitsForPos <= 0
  double truncateFrac = 1.0;
};

} // namespace eicrecon
