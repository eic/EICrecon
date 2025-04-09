// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Simon Gardner
//
// Combine pulses into a larger pulse if they are within a certain time of each other

#include <unordered_map>
#include <vector>

#include "PulseCombiner.h"
#include <algorithms/geo.h>


namespace eicrecon {

void PulseCombiner::init() {

  // Get the detector readout and set CellID bit mask if set
  if(!(m_cfg.readout.empty() && m_cfg.combine_field.empty())) {
    try {
      auto detector = algorithms::GeoSvc::instance().detector();
      auto id_spec = detector->readout(m_cfg.readout).idSpec();
      auto id_dec = id_spec.decoder();
      m_detector_bitmask = 0;
      
      for(auto & field : id_spec.fields()) {
        // Get the field name
        std::string field_name = field.first;
        // Check if the field is the one we want to combine
        m_detector_bitmask |= id_spec.field(field_name)->mask();
        if(field_name == m_cfg.combine_field) {
          break;
        }
      }

    } catch (...) {
      error("Failed set bitshift for detector {} with segmentation id {}", m_cfg.readout, m_cfg.combine_field);
      throw std::runtime_error("Failed to load ID decoder");
    }
  }

}

void PulseCombiner::process(const PulseCombiner::Input& input,
                            const PulseCombiner::Output& output) const {
  const auto [inPulses] = input;
  auto [outPulses]      = output;

  // Create map containing vector of pulses from each CellID
  std::map<uint64_t, std::vector<edm4hep::TimeSeries>> cell_pulses;
  for (const edm4hep::TimeSeries& pulse: *inPulses) {
    uint64_t shiftedCellID = pulse.getCellID() & m_detector_bitmask;
    cell_pulses[shiftedCellID].push_back(pulse);
  }

  // Loop over detector elements and combine pulses
  for (const auto& [cellID, pulses] : cell_pulses) {
    if(pulses.size() == 1) {
      outPulses->push_back(pulses.at(0).clone());
      debug("CellID {} has only one pulse, no combination needed", cellID);
    }
    else{
      std::vector<std::vector<edm4hep::TimeSeries>> clusters = clusterPulses(pulses);
      for (auto cluster: clusters) {
        // Clone the first pulse in the cluster
        auto sum_pulse = outPulses->create();
        sum_pulse.setCellID(cluster[0].getCellID());
        sum_pulse.setInterval(cluster[0].getInterval());
        sum_pulse.setTime(cluster[0].getTime());

        auto newPulse = sumPulses(cluster);
        for(auto pulse : newPulse) {
          sum_pulse.addToAmplitude(pulse);
        }
      }
      debug("CellID {} has {} pulses, combined into {} clusters", cellID, pulses.size(), clusters.size());
    }

  }

} // PulseCombiner:process

std::vector<std::vector<edm4hep::TimeSeries>> PulseCombiner::clusterPulses(const std::vector<edm4hep::TimeSeries> pulses) const {

  // Clone the pulses array of pointers so they aren't const
  std::vector<edm4hep::TimeSeries> ordered_pulses {pulses};

  // Sort pulses by time, greaty simplifying the combination process
  std::sort(ordered_pulses.begin(), ordered_pulses.end(), [](edm4hep::TimeSeries a, edm4hep::TimeSeries b) {
    return a.getTime() < b.getTime();
  });

  // Create vector of pulses
  std::vector<std::vector<edm4hep::TimeSeries>> cluster_pulses;
  float clusterEndTime = 0;
  bool  makeNewPulse = true;
  // Create clusters of pulse indices which overlap with at least the minimum separation
  for (auto pulse: ordered_pulses) {
    float pulseStartTime = pulse.getTime();
    float pulseEndTime = pulse.getTime() + pulse.getInterval()*pulse.getAmplitude().size();
    if(!makeNewPulse) {
      if (pulseStartTime < clusterEndTime + m_cfg.minimum_separation) {
        cluster_pulses.back().push_back(pulse);
        clusterEndTime = std::max(clusterEndTime, pulseEndTime);
      } else {
        makeNewPulse = true;
      }
    }
    if(makeNewPulse) {
      cluster_pulses.push_back({pulse});
      clusterEndTime = pulseEndTime;
      makeNewPulse = false;
    }
  }

  return cluster_pulses;

} // PulseCombiner::clusterPulses

std::vector<float> PulseCombiner::sumPulses(const std::vector<edm4hep::TimeSeries> pulses) const {

  // Find maximum time of pulses in cluster
  float maxTime = 0;
  for (auto pulse: pulses) {
    maxTime = std::max(maxTime, pulse.getTime() + pulse.getInterval()*pulse.getAmplitude().size());
  }

  //Calculate maxTime in interval bins
  int maxStep = std::round((maxTime - pulses[0].getTime()) / pulses[0].getInterval());

  std::vector<float> newPulse(maxStep, 0.0);

  for(auto pulse: pulses) {
    //Calculate start and end of pulse in interval bins
    int startStep = (pulse.getTime() - pulses[0].getTime())/pulse.getInterval();
    int pulseSize = pulse.getAmplitude().size();
    int endStep = startStep + pulseSize;
    for(int i = startStep; i < endStep; i++) {
      // Add pulse values to new pulse
      newPulse[i] += pulse.getAmplitude()[i - startStep];
    }
  }

  return newPulse;
} // PulseCombiner::sumPulses

} // namespace eicrecon
