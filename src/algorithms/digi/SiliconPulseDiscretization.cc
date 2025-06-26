// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Chun Yuen Tsang
//
// Creates a discrete pulse from a continuous pulse
//

#include <DDRec/CellIDPositionConverter.h>
#include <podio/RelationRange.h>
#include <cmath>
#include <gsl/pointers>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "SiliconPulseDiscretization.h"
// use TGraph for interpolation
#include "TGraph.h"

namespace eicrecon {

void SiliconPulseDiscretization::init() {}

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
  const auto [inPulses] = input;
  auto [outPulses]      = output;

  // sometimes the first hit arrives early, but the last hit arrive very late
  // there is a lot of nothing in between
  // If we loop through those time interval with nothing in it, the creation of outPulses will take forever
  // Speeds things up by denoting which EICROCcycle contains pulse information
  // And only focus on those cycles
  std::unordered_map<dd4hep::rec::CellID, std::pair<TGraph, std::unordered_set<int>>>
      graphAndCycle4Cells;

  for (const auto& pulse : *inPulses) {

    auto cellID   = pulse.getCellID();
    auto time     = pulse.getTime();
    auto interval = pulse.getInterval();

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
      for (double currTime = startTime; currTime < startTime + m_cfg.EICROC_period;
           currTime += m_cfg.local_period)
        outPulse.addToAdcCounts(this->_interpolateOrZero(graph, currTime, tMin, tMax));
    }
  }
} // SiliconPulseDiscretization:process
} // namespace eicrecon
