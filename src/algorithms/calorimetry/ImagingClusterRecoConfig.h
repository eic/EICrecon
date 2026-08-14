// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Sylvester Joosten, Whitney Armstrong

#pragma once

#include <iostream>
#include <limits>
#include <sstream>
#include <string>

namespace eicrecon {

/// A self-contained recipe for estimating a cluster's position from its hits.
/// Both positionSource and positionCompareSource in ImagingClusterRecoConfig
/// are independent instances of this same struct, on equal footing -- there is
/// no special-cased "default" algorithm. The recipe always averages the
/// energy-weighted eta/phi of the top hits (by energy) within a layer range;
/// tune maxLayersForPos, averagingMode, numHitsForPos, and truncateFrac to get
/// anything from "highest single hit in the first few layers" to "mean over
/// all hits in all layers" (the latter is the default).
struct PositionEstimatorConfig {
  //! max layer number included in position estimation. Defaults to effectively
  //! unlimited (all layers).
  int maxLayersForPos = std::numeric_limits<int>::max();

  //! selects how many of the highest-energy hits (within maxLayersForPos) are
  //! averaged to compute the position:
  //!   - fixedCount:    average exactly numHitsForPos hits (or fewer, if the
  //!                    cluster has fewer hits within maxLayersForPos)
  //!   - truncatedMean: average the top truncateFrac fraction of hits by
  //!                    energy, rounded down, with a minimum of 1 hit
  enum class EAveragingMode { fixedCount = 0, truncatedMean = 1 } averagingMode =
      EAveragingMode::truncatedMean;

  //! number of hits to average for position; only used when
  //! averagingMode == fixedCount
  int numHitsForPos = 1;

  //! fraction of hits (within maxLayersForPos) to use; only used when
  //! averagingMode == truncatedMean. Defaults to 1.0, i.e. average all hits.
  double truncateFrac = 1.0;

  friend bool operator==(const PositionEstimatorConfig& a, const PositionEstimatorConfig& b) {
    return a.maxLayersForPos == b.maxLayersForPos && a.averagingMode == b.averagingMode &&
           a.numHitsForPos == b.numHitsForPos && a.truncateFrac == b.truncateFrac;
  }
};

inline std::ostream& operator<<(std::ostream& out, const PositionEstimatorConfig::EAveragingMode& m) {
  switch (m) {
    case PositionEstimatorConfig::EAveragingMode::fixedCount: out << "fixedCount"; break;
    case PositionEstimatorConfig::EAveragingMode::truncatedMean: out << "truncatedMean"; break;
    default: out.setstate(std::ios::failbit);
  }
  return out;
}

inline std::istream& operator>>(std::istream& in, PositionEstimatorConfig::EAveragingMode& m) {
  std::string s;
  in >> s;
  if (s == "fixedCount" || s == "0") {
    m = PositionEstimatorConfig::EAveragingMode::fixedCount;
  } else if (s == "truncatedMean" || s == "1") {
    m = PositionEstimatorConfig::EAveragingMode::truncatedMean;
  } else {
    in.setstate(std::ios::failbit);
  }
  return in;
}

/// Serialized format: maxLayersForPos:averagingMode:numHitsForPos:truncateFrac
/// Example: "6:fixedCount:1:1.0" (top 1 hit within the first 6 layers)
/// Example: "999:truncatedMean:1:0.2" (top 20% of hits by energy, all layers)
inline std::ostream& operator<<(std::ostream& out, const PositionEstimatorConfig& e) {
  out << e.maxLayersForPos << ":" << e.averagingMode << ":" << e.numHitsForPos << ":"
      << e.truncateFrac;
  return out;
}

inline std::istream& operator>>(std::istream& in, PositionEstimatorConfig& e) {
  std::string token;
  in >> token;
  for (char& c : token) {
    if (c == ':') c = ' ';
  }
  std::istringstream ss(token);
  PositionEstimatorConfig tmp;
  if (!(ss >> tmp.maxLayersForPos >> tmp.averagingMode >> tmp.numHitsForPos >> tmp.truncateFrac)) {
    in.setstate(std::ios::failbit);
    return in;
  }
  e = tmp;
  return in;
}

struct ImagingClusterRecoConfig {

  int trackStopLayer = 9;

  //! recipe used for the full 3D position (x, y, z + covariance) if it agrees
  //! with positionCompareSource (or always, if positionMaxDphi < 0).
  //! Defaults to averaging all hits in all layers, i.e. a plain energy-weighted
  //! mean -- identical to the previous unconditional behavior.
  PositionEstimatorConfig positionSource{};

  //! recipe used for the full 3D position if positionSource disagrees with it
  //! in phi (only evaluated when positionMaxDphi >= 0). Independent, equally
  //! configurable recipe -- not a special case of positionSource.
  PositionEstimatorConfig positionCompareSource{};

  //! if >= 0, compare positionSource and positionCompareSource in azimuthal
  //! angle: if they agree (within this tolerance), use positionSource for the
  //! full 3D position (x, y, z, and covariance); if they disagree, use
  //! positionCompareSource instead. The two are never mixed.
  //! if < 0, positionCompareSource is not computed and positionSource is
  //! always used.
  double positionMaxDphi = -1;
};

} // namespace eicrecon

