// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Simon Gardner
//
// Combine pulses into a larger pulse if they are within a certain time of each other

#include <Evaluator/DD4hepUnits.h>
#include <unordered_map>
#include <vector>

#include "PulseCombiner.h"

namespace eicrecon {

void PulseCombiner::init() {
  m_minimum_separation = m_cfg.minimum_separation;
}

void PulseCombiner::process(const PulseCombiner::Input& input,
                            const PulseCombiner::Output& output) const {
  const auto [inPulses] = input;
  auto [outPulses]      = output;

  // Vector of available pulses
  std::vector<int> avaliable_pulses(inPulses->size(),1);

  // Create clusters of pulse indices which overlap with at least the minimum separation
  // TODO: Check CellID for each pulse to ensure they are from the same detector element
  std::vector<std::vector<int>> pulse_clusters;
  for (int i = 0; i < inPulses->size(); i++) {
    if (!avaliable_pulses[i]) continue;
    const auto& pulse = inPulses->at(i);
    bool added = false;
    for (auto& cluster : pulse_clusters) {
      for (int j : cluster) {
        if (pulse_times[i].first - pulse_times[j].second < m_minimum_separation) {
          cluster.push_back(i);
          added = true;
          break;
        }
      }
    }
    if (!added) {
      pulse_clusters.push_back({i});
    }
  } 

  // Create new pulses from the clusters
  for (const auto& cluster : pulse_clusters) {
    double start_time = pulse_times[cluster[0]].first;
    double stop_time = pulse_times[cluster[0]].second;
    for (int i = 1; i < cluster.size(); i++) {
      if (pulse_times[cluster[i]].first < start_time) {
        start_time = pulse_times[cluster[i]].first;
      }
      if (pulse_times[cluster[i]].second > stop_time) {
        stop_time = pulse_times[cluster[i]].second;
      }
    }
    auto pulse = outPulses->create();
    pulse.setTime(start_time);
    pulse.setInterval(stop_time - start_time);
    pulse.setAmplitude(0.0);
  }

} // PulseCombiner:process
} // namespace eicrecon
