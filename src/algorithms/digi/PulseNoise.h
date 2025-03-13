// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul, Chun Yuen Tsang, Prithwish Tribedy
// Special Acknowledgement: Kolja Kauder
//
// Convert energy deposition into ADC pulses
// ADC pulses are assumed to follow the shape of landau function

#pragma once

#include <DDRec/CellIDPositionConverter.h>
#include <DDDigi/noise/FalphaNoise.h>
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

#include "algorithms/digi/PulseNoiseConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using PulseNoiseAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::TimeSeriesCollection>,
                          algorithms::Output<edm4hep::TimeSeriesCollection>>;

class PulseNoise : public PulseNoiseAlgorithm,
                               public WithPodConfig<PulseNoiseConfig> {

public:
  PulseNoise(std::string_view name)
      : PulseNoiseAlgorithm{name, {"RawHits"}, {"OutputPulses"}, {}} {}
  virtual void init() final;
  void process(const Input&, const Output&);

private:

  std::default_random_engine generator; // TODO: need something more appropriate here
  dd4hep::detail::FalphaNoise m_noise;
  size_t m_poles    = 5;
  double m_varience = 1.0;
  double m_alpha    = 0.5;
  double m_scale    = 1000.0;


};

} // namespace eicrecon
