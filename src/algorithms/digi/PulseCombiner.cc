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

  // Create map containing vector of pulses from each CellID
  std::unordered_map<uint64_t, std::vector<edm4hep::TimeSeries*>> cell_pulses;
  for (auto pulse: *inPulses) {
    cell_pulses[pulse.getCellID()].push_back(&pulse);
  }

  // Loop over detector elements and combine pulses
  for (const auto& [cellID, pulses] : cell_pulses) {
    if(pulses.size() == 1) {
      outPulses->push_back(pulses.at(0)->clone());
    }
    else{
      std::vector<std::vector<edm4hep::TimeSeries*>> clusters = clusterPulses(pulses);
      for (auto cluster: clusters) {
        outPulses->push_back(sumPulses(cluster));
      }      
    }
  }

} // PulseCombiner:process

std::vector<std::vector<edm4hep::TimeSeries*>> PulseCombiner::clusterPulses(const std::vector<edm4hep::TimeSeries*>& pulses) const {

  //Clone the pulses aray of pointers so they aren't const
  std::vector<edm4hep::TimeSeries*> ordered_pulses;
  for(auto pulse : pulses) {
    ordered_pulses.push_back(pulse);
  }

  // Sort pulses by time, greaty simplifying the combination process
  std::sort(ordered_pulses.begin(), ordered_pulses.end(), [](edm4hep::TimeSeries* a, edm4hep::TimeSeries* b) {
    return a->getTime() < b->getTime();
  });

  // Create vector of pulses
  std::vector<std::vector<edm4hep::TimeSeries*>> cluster_pulses;
  float clusterEndTime = 0;
  bool isNewPulse = true;
  // Create clusters of pulse indices which overlap with at least the minimum separation
  for (auto pulse: ordered_pulses) {
    float pulseStartTime = pulse->getTime();
    float pulseEndTime = pulse->getTime() + pulse->getInterval()*pulse->getAmplitude().size();

    if(!isNewPulse) {
      if (pulseStartTime < clusterEndTime + m_minimum_separation) {
        cluster_pulses.back().push_back(pulse);
        clusterEndTime = std::max(clusterEndTime, pulseEndTime);
      } else {
        isNewPulse = true;
      }
    }
    if(isNewPulse) {
      cluster_pulses.push_back({pulse});
      clusterEndTime = pulseEndTime;
      isNewPulse = false;
    }
  }
  
  return cluster_pulses;

} // PulseCombiner::clusterPulses

edm4hep::MutableTimeSeries PulseCombiner::sumPulses(const std::vector<edm4hep::TimeSeries*>& pulses) const {
  // Clone the first pulse in the cluster
  edm4hep::MutableTimeSeries sum_pulse(pulses.at(0)->getCellID(),pulses.at(0)->getTime(), pulses.at(0)->getInterval());
  
  // Find maximum time of pulses in cluster
  float maxTime = 0;
  for (auto pulse: pulses) {
    maxTime = std::max(maxTime, pulse->getTime() + pulse->getInterval()*pulse->getAmplitude().size());
  }

  //Calculate maxTime in interval bins
  maxTime = maxTime - pulses.at(0)->getTime();
  int maxStep = maxTime/pulses.at(0)->getInterval();

  std::vector<float> newPulse(maxStep, 0.0);
  
  for(auto pulse: pulses) {
    //Calculate start and end of pulse in interval bins
    int startStep = (pulse->getTime() - pulses.at(0)->getTime())/pulse->getInterval();
    int endStep = startStep + pulse->getAmplitude().size();
    for(int i = 0; i < newPulse.size(); i++) {
      // Add pulse values to new pulse
      if(i >= startStep && i < endStep){
        newPulse[startStep + i] += pulse->getAmplitude()[i - startStep];
      } else { // Interpolate first and last two values to extrapolate over 0
        float contribution = 0;
        if(i < startStep) {
          contribution = pulse->getAmplitude()[0] + (pulse->getAmplitude()[1] - pulse->getAmplitude()[0])*(i - startStep);
        } else if(i >= endStep) {
          contribution = pulse->getAmplitude()[endStep - startStep - 1] + (pulse->getAmplitude()[endStep - startStep - 1] - pulse->getAmplitude()[endStep - startStep - 2])*(i - endStep);
        }
        if(contribution>0){
          newPulse[i] += contribution;
        }
      }
    }
  }
    
  for(auto signal: newPulse) {
    sum_pulse.addToAmplitude(signal);
  }

  return sum_pulse;
} // PulseCombiner::sumPulses

} // namespace eicrecon
