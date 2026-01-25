// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Simon Gardner
//
// Combine pulses into a larger pulse if they are within a certain time of each other

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <DDSegmentation/BitFieldCoder.h>
#include <algorithms/geo.h>
#include <edm4hep/MCParticle.h>
#include <edm4hep/SimCalorimeterHit.h>
#include <edm4hep/SimTrackerHit.h>
#include <edm4hep/Vector3f.h>
#include <podio/RelationRange.h>
#include <algorithm>
#include <cmath>
#include <gsl/pointers>
#include <map>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <vector>

#include "PulseCombiner.h"

namespace eicrecon {

void PulseCombiner::init() {

  // Get the detector readout and set CellID bit mask if set
  if (!(m_cfg.readout.empty() && m_cfg.combine_field.empty())) {
    try {
      auto detector      = algorithms::GeoSvc::instance().detector();
      auto id_spec       = detector->readout(m_cfg.readout).idSpec();
      m_detector_bitmask = 0;

      for (const auto& field : id_spec.fields()) {
        // Get the field name
        std::string field_name = field.first;
        // Check if the field is the one we want to combine
        m_detector_bitmask |= id_spec.field(field_name)->mask();
        if (field_name == m_cfg.combine_field) {
          break;
        }
      }

    } catch (...) {
      error("Failed set bitshift for detector {} with segmentation id {}", m_cfg.readout,
            m_cfg.combine_field);
      throw std::runtime_error("Failed to load ID decoder");
    }
  }
}

void PulseCombiner::process(const PulseCombiner::Input& input,
                            const PulseCombiner::Output& output) const {
  const auto [inPulses] = input;
  auto [outPulses]      = output;

  // Create map containing vector of pulses from each CellID
  std::map<uint64_t, std::vector<PulseType>> cell_pulses;
  for (const PulseType& pulse : *inPulses) {
    uint64_t shiftedCellID = pulse.getCellID() & m_detector_bitmask;
    cell_pulses[shiftedCellID].push_back(pulse);
  }

  // Loop over detector elements and combine pulses
  for (const auto& [cellID, pulses] : cell_pulses) {
    if (pulses.size() == 1) {
      outPulses->push_back(pulses.at(0).clone());
      debug("CellID {} has only one pulse, no combination needed", cellID);
    } else {
      std::vector<std::vector<PulseType>> clusters = clusterPulses(pulses);
      for (auto cluster : clusters) {
        // Clone the first pulse in the cluster
        auto sum_pulse = outPulses->create();
        sum_pulse.setCellID(cluster[0].getCellID());
        sum_pulse.setInterval(cluster[0].getInterval());
        sum_pulse.setTime(cluster[0].getTime());

        auto newPulse = sumPulses(cluster);
        for (auto pulse : newPulse) {
          sum_pulse.addToAmplitude(pulse);
        }

#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 1)
        // Sum the pulse array
        float integral = std::accumulate(newPulse.begin(), newPulse.end(), 0.0F);
        sum_pulse.setIntegral(integral);
        sum_pulse.setPosition(edm4hep::Vector3f(
            cluster[0].getPosition().x, cluster[0].getPosition().y, cluster[0].getPosition().z));
        for (auto pulse : cluster) {
          sum_pulse.addToPulses(pulse);
          for (auto particle : pulse.getParticles()) {
            sum_pulse.addToParticles(particle);
          }
          for (auto hit : pulse.getTrackerHits()) {
            sum_pulse.addToTrackerHits(hit);
          }
          for (auto hit : pulse.getCalorimeterHits()) {
            sum_pulse.addToCalorimeterHits(hit);
          }
        }
#endif
      }
      debug("CellID {} has {} pulses, combined into {} clusters", cellID, pulses.size(),
            clusters.size());
    }
  }

} // PulseCombiner:process

std::vector<std::vector<PulseType>>
PulseCombiner::clusterPulses(const std::vector<PulseType> pulses) const {

  // Clone the pulses array of pointers so they aren't const
  std::vector<PulseType> ordered_pulses{pulses};

  // Sort pulses by time, greaty simplifying the combination process
  std::ranges::sort(ordered_pulses,
                    [](PulseType a, PulseType b) { return a.getTime() < b.getTime(); });

  // Create vector of pulses
  std::vector<std::vector<PulseType>> cluster_pulses;
  float clusterEndTime = 0;
  bool makeNewPulse    = true;
  // Create clusters of pulse indices which overlap with at least the minimum separation
  for (auto pulse : ordered_pulses) {
    float pulseStartTime = pulse.getTime();
    float pulseEndTime   = pulse.getTime() + pulse.getInterval() * pulse.getAmplitude().size();
    if (!makeNewPulse) {
      if (pulseStartTime < clusterEndTime + m_cfg.minimum_separation) {
        cluster_pulses.back().push_back(pulse);
        clusterEndTime = std::max(clusterEndTime, pulseEndTime);
      } else {
        makeNewPulse = true;
      }
    }
    if (makeNewPulse) {
      cluster_pulses.push_back({pulse});
      clusterEndTime = pulseEndTime;
      makeNewPulse   = false;
    }
  }

  return cluster_pulses;

} // PulseCombiner::clusterPulses

std::vector<float> PulseCombiner::sumPulses(const std::vector<PulseType> pulses) {

  // Find maximum time of pulses in cluster
  float maxTime = 0;
  for (auto pulse : pulses) {
    maxTime =
        std::max(maxTime, pulse.getTime() + pulse.getInterval() * pulse.getAmplitude().size());
  }

  //Calculate maxTime in interval bins
  int maxStep = std::round((maxTime - pulses[0].getTime()) / pulses[0].getInterval());

  std::vector<float> newPulse(maxStep, 0.0);

  for (auto pulse : pulses) {
    //Calculate start and end of pulse in interval bins
    int startStep = (pulse.getTime() - pulses[0].getTime()) / pulse.getInterval();
    int pulseSize = pulse.getAmplitude().size();
    int endStep   = startStep + pulseSize;
    for (int i = startStep; i < endStep; i++) {
      // Add pulse values to new pulse
      newPulse[i] += pulse.getAmplitude()[i - startStep];
    }
  }

  return newPulse;
} // PulseCombiner::sumPulses

} // namespace eicrecon
