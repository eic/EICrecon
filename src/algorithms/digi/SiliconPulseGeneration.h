// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul, Chun Yuen Tsang, Prithwish Tribedy
// Special Acknowledgement: Kolja Kauder
//
// Convert energy deposition into ADC pulses
// ADC pulses are assumed to follow the shape of landau function

#pragma once

#include <DDRec/CellIDPositionConverter.h>
#include <RtypesCore.h>
#include <TMath.h>
#include <algorithms/algorithm.h>
#include <edm4hep/TimeSeriesCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "algorithms/digi/SiliconPulseGenerationConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using SiliconPulseGenerationAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::SimTrackerHitCollection>,
                          algorithms::Output<edm4hep::TimeSeriesCollection>>;

class SiliconPulseGeneration : public SiliconPulseGenerationAlgorithm,
                               public WithPodConfig<SiliconPulseGenerationConfig> {

public:
  SiliconPulseGeneration(std::string_view name)
      : SiliconPulseGenerationAlgorithm{name, {"RawHits"}, {"OutputPulses"}, {}} {}
  virtual void init() final;
  void process(const Input&, const Output&) const final;

private:

  std::shared_ptr<SignalPulse> m_pulse;
  int m_max_time_bins = 10000;

};

} // namespace eicrecon
