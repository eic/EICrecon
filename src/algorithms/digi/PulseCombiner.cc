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
  const auto [inpulses] = input;
  auto [outPulses]      = output;

  //Create array of pulse start and stop times
  std::vector<std::pair<double,double>> pulse_times;
  for (const auto& pulse : *inpulses) {
    double start_time = pulse.getTime();
    double stop_time = start_time + pulse.getInterval();
    pulse_times.push_back(std::make_pair(start_time,stop_time));
  }

  //Loop over all pulses tagging groups which need to be combined
  std::vector<std::vector<int>> pulse_groups;
  for (int i=0; i<pulse_times.size(); i++){
    std::vector<int> group;
    group.push_back(i);
    for (int j=i+1; j<pulse_times.size(); j++){
      if (pulse_times[j].first - pulse_times[i].second < m_minimum_separation){
        group.push_back(j);
      }
    }
    pulse_groups.push_back(group);
  }

} // PulseCombiner:process
} // namespace eicrecon
