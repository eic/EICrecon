// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Simon Gardner
//
// Adds noise to a time series pulse
//

#pragma once

#include <TGraph.h>
#include <algorithms/algorithm.h>
#include <edm4hep/RawTimeSeriesCollection.h>
#include <edm4hep/TimeSeriesCollection.h>
#include <string>
#include <string_view>

#include "algorithms/digi/SiliconPulseDiscretizationConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using SiliconPulseDiscretizationAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::TimeSeriesCollection>,
                          algorithms::Output<edm4hep::RawTimeSeriesCollection>>;

class SiliconPulseDiscretization : public SiliconPulseDiscretizationAlgorithm,
                               public WithPodConfig<SiliconPulseDiscretizationConfig> {

public:
  SiliconPulseDiscretization(std::string_view name)
      : SiliconPulseDiscretizationAlgorithm{name, {"OutputPulses"}, {"DiscretePulses"}, {}} {}
  virtual void init() final;
  void process(const Input&, const Output&) const;

private:
  double _interpolateOrZero(const TGraph &graph, double t,
                            double tMin, double tMax) const;

};

} // namespace eicrecon
