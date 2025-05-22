// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Chun Yuen Tsang
//
// Creates a discrete pulse from a continuous pulse
//

#include <DDRec/CellIDPositionConverter.h>
#include <climits>
#include <cmath>
#include <gsl/pointers>
#include <podio/RelationRange.h>
#include <unordered_map>

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

  std::unordered_map<dd4hep::rec::CellID, TGraph> Graph4Cells;

  for (const auto& pulse : *inPulses) {

    auto cellID   = pulse.getCellID();
    auto time     = pulse.getTime();
    auto interval = pulse.getInterval();

    // one TGraph per pulse
    // Interpolate the pulse with TGraph
    auto& graph = Graph4Cells[cellID];
    for (unsigned int i = 0; i < pulse.getAmplitude().size(); i++) {
      auto currTime = time + i * interval;
      graph.SetPoint(graph.GetN(), currTime + m_cfg.global_offset, pulse.getAmplitude()[i]);
    }
  }

  // sort all pulses data points to avoid TGraph::Eval giving nan due to non-monotonic data
  for (auto& [cellID, graph] : Graph4Cells) {
    graph.Sort();
  }

  // sum all digitized pulses
  for (const auto& [cellID, graph] : Graph4Cells) {
    double tMin;
    double tMax;
    double temp = NAN; // do not use
    graph.ComputeRange(tMin, temp, tMax, temp);

    // time beings at an EICROC cycle
    double currTime = std::floor(tMin / m_cfg.EICROC_period) * m_cfg.EICROC_period;
    // ensure that the outPulse -> create is called in the first cycle
    int iEICRocCycle = INT_MIN;

    edm4hep::MutableRawTimeSeries outPulse;
    for (; currTime <= tMax; currTime += m_cfg.local_period) {
      // find current EICROC cycle NO to see if we arrive at the next cycle
      int curriEICRocCycle = std::floor(currTime / m_cfg.EICROC_period);
      if (curriEICRocCycle != iEICRocCycle) {
        // new pulse for each EICROC cycle
        iEICRocCycle = curriEICRocCycle;
        outPulse     = outPulses->create();
        outPulse.setCellID(cellID);
        outPulse.setInterval(m_cfg.local_period);
        outPulse.setTime(iEICRocCycle * m_cfg.EICROC_period);
      }
      outPulse.addToAdcCounts(this->_interpolateOrZero(graph, currTime, tMin, tMax));
    }
  }
} // SiliconPulseDiscretization:process
} // namespace eicrecon
