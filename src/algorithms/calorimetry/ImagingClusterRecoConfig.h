// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Sylvester Joosten, Whitney Armstrong

#pragma once

#include <iostream>
#include <string>

namespace eicrecon {

struct ImagingClusterRecoConfig {

  int trackStopLayer = 9;

  //! if true, override cluster position using the top highest-energy hits
  //! within layers <= maxLayersForPos, averaged in eta-phi space
  bool usePositionOfHighestEnergyHit = false;

  //! max layer number included in position estimation
  int maxLayersForPos = 6;

  //! selects how many of the highest-energy hits (within maxLayersForPos) are
  //! averaged to compute the overridden position:
  //!   - fixedCount:    average exactly numHitsForPos hits (or fewer, if the
  //!                    cluster has fewer hits within maxLayersForPos)
  //!   - truncatedMean: average the top truncateFrac fraction of hits by
  //!                    energy, rounded down, with a minimum of 1 hit
  enum class EPositionAveragingMode { fixedCount = 0, truncatedMean = 1 } positionAveragingMode =
      EPositionAveragingMode::fixedCount;

  //! number of hits to average for position; only used when
  //! positionAveragingMode == fixedCount
  int numHitsForPos = 1;

  //! fraction of hits (within maxLayersForPos) to use; only used when
  //! positionAveragingMode == truncatedMean
  double truncateFrac = 1.0;
};

inline std::istream& operator>>(std::istream& in,
                                ImagingClusterRecoConfig::EPositionAveragingMode& mode) {
  std::string s;
  in >> s;
  if (s == "fixedCount" || s == "0") {
    mode = ImagingClusterRecoConfig::EPositionAveragingMode::fixedCount;
  } else if (s == "truncatedMean" || s == "1") {
    mode = ImagingClusterRecoConfig::EPositionAveragingMode::truncatedMean;
  } else {
    in.setstate(std::ios::failbit);
  }
  return in;
}

inline std::ostream& operator<<(std::ostream& out,
                                const ImagingClusterRecoConfig::EPositionAveragingMode& mode) {
  switch (mode) {
    case ImagingClusterRecoConfig::EPositionAveragingMode::fixedCount:
      out << "fixedCount";
      break;
    case ImagingClusterRecoConfig::EPositionAveragingMode::truncatedMean:
      out << "truncatedMean";
      break;
    default:
      out.setstate(std::ios::failbit);
  }
  return out;
}

} // namespace eicrecon
