// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Chun Yuen Tsang
//
// Creates a discrete pulse from a continuous pulse
//

#pragma once

#include <TGraph.h>
#include <algorithms/algorithm.h>
#include <edm4hep/RawTimeSeriesCollection.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/SimPulseCollection.h>
#include <string>
#include <string_view>

#include "algorithms/digi/SiliconPulseDiscretizationConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using PulseType = edm4eic::SimPulse;

using SiliconPulseDiscretizationAlgorithm =
    algorithms::Algorithm<algorithms::Input<PulseType::collection_type>,
                          algorithms::Output<edm4hep::RawTimeSeriesCollection>>;

class SiliconPulseDiscretization : public SiliconPulseDiscretizationAlgorithm,
                                   public WithPodConfig<SiliconPulseDiscretizationConfig> {

public:
  SiliconPulseDiscretization(std::string_view name)
      : SiliconPulseDiscretizationAlgorithm{name, {"OutputPulses"}, {"DiscretePulses"}, {}} {}
  virtual void init() final;
  void process(const Input&, const Output&) const;

private:
  double _interpolateOrZero(const TGraph& graph, double t, double tMin, double tMax) const;
};

} // namespace eicrecon
