// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/SimPulseCollection.h>
#include <edm4eic/RawHGCROCHitCollection.h>
#include <edm4eic/HGCROCSample.h>
#include <stdint.h>
#include <functional>
#include <string>
#include <string_view>

#include "algorithms/digi/PulseDigiConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using PulseDigiAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::SimPulseCollection>,
    algorithms::Output<edm4eic::RawHGCROCHitCollection>>;

class PulseDigi : public PulseDigiAlgorithm, public WithPodConfig<PulseDigiConfig> {

public:
  PulseDigi(std::string_view name)
      : PulseDigiAlgorithm{name, {"InputPulses"}, {"OutputRawHGCROCHits"}, {}} {}
  virtual void init() final;
  void process(const Input&, const Output&) const;

private:
  edm4eic::HGCROCSample sample;

private:
  double get_crossing_time(double thres, double dt, double t, 
			   double amp1, double amp2) const;
};

} // namespace eicrecon
