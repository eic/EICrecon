// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Chun Yuen Tsang
//
// Creates a discrete pulse from a continuous pulse
//

#include <DDRec/CellIDPositionConverter.h>
#include <edm4hep/MCParticle.h>
#include <edm4hep/SimCalorimeterHit.h>
#include <edm4hep/SimTrackerHit.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <cmath>
#include <cstddef>
#include <functional>
#include <iterator>
#include <set>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "SiliconPulseDiscretization.h"
// use TGraph for interpolation
#include "TGraph.h"

namespace eicrecon {

void SiliconPulseDiscretization::init() {}

namespace {

  // A leaf pulse has no parent pulses, e.g. one pulse per sim hit from PulseGeneration
  void collectLeafPulses(const PulseType& pulse, std::vector<PulseType>& leaves) {
    if (pulse.pulses_size() == 0) {
      leaves.push_back(pulse);
      return;
    }
    for (const auto& parent : pulse.getPulses()) {
      collectLeafPulses(parent, leaves);
    }
  }

  // Whether the samples of a pulse, shifted by offset, fall in [start, start + period)
  bool overlapsWindow(const PulseType& pulse, double offset, double start, double period) {
    double first = pulse.getTime() + offset;
    double last  = first;
    if (pulse.getAmplitude().size() > 1) {
      last += (pulse.getAmplitude().size() - 1) * pulse.getInterval();
    }
    return first < start + period && last >= start;
  }

} // namespace

void SiliconPulseDiscretization::_fillTruth(edm4eic::MutableSimPulse& truth,
                                            const std::vector<PulseType>& cellPulses,
                                            double startTime) const {
  // Truth for one EICROC cycle: the sim hits whose leaf pulses lie in this cycle.
  // The time series samples the pulses shifted by global_offset, so match with that
  // shift first. A cycle is made active by the unshifted sample times, so fall back to
  // unshifted leaf times and then to whole input pulses: every time series gets truth.
  const double period = m_cfg.EICROC_period;

  std::vector<std::vector<PulseType>> leaves;
  for (const auto& pulse : cellPulses) {
    collectLeafPulses(pulse, leaves.emplace_back());
  }

  std::set<podio::ObjectID> seen_pulses;
  std::set<podio::ObjectID> seen_tracker_hits;
  std::set<podio::ObjectID> seen_calorimeter_hits;
  std::set<podio::ObjectID> seen_particles;
  auto add_leaf = [&](const PulseType& pulse, const PulseType& leaf) {
    if (seen_pulses.insert(pulse.getObjectID()).second) {
      truth.addToPulses(pulse);
    }
    for (const auto& hit : leaf.getTrackerHits()) {
      if (seen_tracker_hits.insert(hit.getObjectID()).second) {
        truth.addToTrackerHits(hit);
      }
    }
    for (const auto& hit : leaf.getCalorimeterHits()) {
      if (seen_calorimeter_hits.insert(hit.getObjectID()).second) {
        truth.addToCalorimeterHits(hit);
      }
    }
    for (const auto& particle : leaf.getParticles()) {
      if (seen_particles.insert(particle.getObjectID()).second) {
        truth.addToParticles(particle);
      }
    }
  };

  auto collect = [&](const std::function<bool(const PulseType&, const PulseType&)>& accept) {
    bool found = false;
    for (std::size_t i = 0; i < cellPulses.size(); ++i) {
      for (const auto& leaf : leaves[i]) {
        if (accept(cellPulses[i], leaf)) {
          add_leaf(cellPulses[i], leaf);
          found = true;
        }
      }
    }
    return found;
  };

  const double offset = m_cfg.global_offset;
  if (collect([&](const PulseType& /* pulse */, const PulseType& leaf) {
        return overlapsWindow(leaf, offset, startTime, period);
      })) {
    return;
  }
  if (collect([&](const PulseType& /* pulse */, const PulseType& leaf) {
        return overlapsWindow(leaf, 0, startTime, period);
      })) {
    return;
  }
  collect([&](const PulseType& pulse, const PulseType& /* leaf */) {
    return overlapsWindow(pulse, 0, startTime, period);
  });
}

double SiliconPulseDiscretization::_interpolateOrZero(const TGraph& graph, double t, double tMin,
                                                      double tMax) const {
  // return 0 if t is outside of tMin - tMax range
  // otherwise, return graph interpolation value
  if (t < tMin || t > tMax) {
    return 0;
  }
  double height = graph.Eval(t, nullptr, "S"); // spline interpolation
  if (!std::isfinite(height)) {
    error("Pulse interpolation returns nan. This happen mostly because there are multiple "
          "pulse height values at the same time. Did you call PulseCombiner?");
  }
  return height;
}

void SiliconPulseDiscretization::process(const SiliconPulseDiscretization::Input& input,
                                         const SiliconPulseDiscretization::Output& output) const {
  const auto [inPulses]      = input;
  auto [outPulses, outTruth] = output;

  // sometimes the first hit arrives early, but the last hit arrive very late
  // there is a lot of nothing in between
  // If we loop through those time interval with nothing in it, the creation of outPulses will take forever
  // Speeds things up by denoting which EICROCcycle contains pulse information
  // And only focus on those cycles
  std::unordered_map<dd4hep::rec::CellID, std::pair<TGraph, std::unordered_set<int>>>
      graphAndCycle4Cells;
  // input pulses of each cell, in input order, for the truth output
  std::unordered_map<dd4hep::rec::CellID, std::vector<PulseType>> pulses4Cells;

  for (const auto& pulse : *inPulses) {

    auto cellID   = pulse.getCellID();
    auto time     = pulse.getTime();
    auto interval = pulse.getInterval();
    pulses4Cells[cellID].push_back(pulse);

    // one TGraph per pulse
    // Interpolate the pulse with TGraph
    auto& graph        = graphAndCycle4Cells[cellID].first;
    auto& activeCycles = graphAndCycle4Cells[cellID].second;
    for (unsigned int i = 0; i < pulse.getAmplitude().size(); i++) {
      auto currTime = time + i * interval;
      // current EICROC cycle
      int EICROCCycle = std::floor(currTime / m_cfg.EICROC_period);
      activeCycles.insert(EICROCCycle);
      graph.SetPoint(graph.GetN(), currTime + m_cfg.global_offset, pulse.getAmplitude()[i]);
    }
  }

  // sort all pulses data points to avoid TGraph::Eval giving nan due to non-monotonic data
  for (auto& [_, graphAndCycle] : graphAndCycle4Cells) {
    graphAndCycle.first.Sort();
    graphAndCycle.first.SetBit(TGraph::kIsSortedX);
  }

  // sum all digitized pulses
  for (const auto& [cellID, graphAndCycle] : graphAndCycle4Cells) {
    const auto& graph       = graphAndCycle.first;
    const auto& activeCycle = graphAndCycle.second;
    double tMin             = NAN;
    double tMax             = NAN;
    double temp             = NAN; // do not use
    graph.ComputeRange(tMin, temp, tMax, temp);

    for (int curriEICRocCycle : activeCycle) {
      // time beings at an EICROC cycle
      double startTime = curriEICRocCycle * m_cfg.EICROC_period;

      auto outPulse = outPulses->create();
      outPulse.setCellID(cellID);
      outPulse.setInterval(m_cfg.local_period);
      outPulse.setTime(startTime);

      // stop at the next cycle
      // NOLINTNEXTLINE(clang-analyzer-security.FloatLoopCounter, security.FloatLoopCounter)
      for (double currTime = startTime; currTime < startTime + m_cfg.EICROC_period;
           currTime += m_cfg.local_period) {
        outPulse.addToAdcCounts(this->_interpolateOrZero(graph, currTime, tMin, tMax));
      }

      // truth for this time series, at the same index in outTruth
      const auto& cellPulses = pulses4Cells.at(cellID);
      auto truth             = outTruth->create();
      truth.setCellID(cellID);
      truth.setInterval(m_cfg.local_period);
      truth.setTime(startTime);
      truth.setPosition(cellPulses.front().getPosition());
      _fillTruth(truth, cellPulses, startTime);
    }
  }
} // SiliconPulseDiscretization:process
} // namespace eicrecon
