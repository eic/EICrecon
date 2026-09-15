// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ePIC Collaboration

#pragma once

#include <map>
#include <numeric>

namespace eicrecon::detail {

/// Add one measurement's truth contribution to a track-level accumulator.
///
/// Energy-deposit weights are preferred when available. If all deposits are
/// zero or unavailable, association counts are used as a fallback. In either
/// case the selected per-measurement weights are normalized to unit sum, so one
/// detector measurement cannot outweigh several independent measurements just
/// because it contains more SimTrackerHit records.
template <typename Key, typename Compare>
void accumulateMeasurementTruthWeights(const std::map<Key, double, Compare>& energyWeights,
                                       const std::map<Key, double, Compare>& countWeights,
                                       std::map<Key, double, Compare>& trackWeights) {
  const auto sumWeights = [](const auto& weights) {
    return std::accumulate(weights.begin(), weights.end(), 0.0,
                           [](double sum, const auto& item) { return sum + item.second; });
  };

  const double energyTotal = sumWeights(energyWeights);
  const auto& weights      = energyTotal > 0.0 ? energyWeights : countWeights;
  const double total       = sumWeights(weights);
  if (total <= 0.0) {
    return;
  }

  for (const auto& [key, contribution] : weights) {
    trackWeights[key] += contribution / total;
  }
}

} // namespace eicrecon::detail
