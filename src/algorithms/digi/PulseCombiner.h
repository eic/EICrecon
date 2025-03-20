// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Simon Gardner
//
// Convert energy deposition into ADC pulses
// ADC pulses are assumed to follow the shape of landau function

#pragma once

#include <algorithms/algorithm.h>
#include <edm4hep/TimeSeriesCollection.h>
#include <memory>
#include <string>
#include <string_view>
#include <edm4eic/unit_system.h>

#include "algorithms/digi/PulseCombinerConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using PulseCombinerAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::TimeSeriesCollection>,
                          algorithms::Output<edm4hep::TimeSeriesCollection>>;

class PulseCombiner : public PulseCombinerAlgorithm,
                               public WithPodConfig<PulseCombinerConfig> {

public:
  PulseCombiner(std::string_view name)
      : PulseCombinerAlgorithm{name, {"InputPulses"}, {"OutputPulses"}, {}} {}
  virtual void init() final;
  void process(const Input&, const Output&) const final;

private:

  std::vector<std::vector<edm4hep::TimeSeries>> clusterPulses(const std::vector<edm4hep::TimeSeries> pulses) const;
  std::vector<float> sumPulses(const std::vector<edm4hep::TimeSeries> pulses) const;
  float m_minimum_separation = 1000*edm4eic::unit::ns;
  uint64_t m_detector_bitmask = 0xFFFFFFFFFFFFFFFF;

};

} // namespace eicrecon
