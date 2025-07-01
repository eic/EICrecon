// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Simon Gardner
//
// Adds noise to a time series pulse
//

#pragma once

#include <DDDigi/noise/FalphaNoise.h>
#include <algorithms/algorithm.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4hep/EventHeaderCollection.h>
#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 1)
#include <edm4eic/SimPulseCollection.h>
#else
#include <edm4hep/TimeSeriesCollection.h>
#endif
#include <string>
#include <string_view>

#include "algorithms/digi/PulseNoiseConfig.h"
#include "algorithms/interfaces/UniqueIDGenSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 1)
using PulseType = edm4eic::SimPulse;
#else
using PulseType = edm4hep::TimeSeries;
#endif

using PulseNoiseAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4hep::EventHeaderCollection, PulseType::collection_type>,
    algorithms::Output<PulseType::collection_type>>;

class PulseNoise : public PulseNoiseAlgorithm, public WithPodConfig<PulseNoiseConfig> {

public:
  PulseNoise(std::string_view name)
      : PulseNoiseAlgorithm{name, {"RawHits"}, {"OutputPulses"}, {}} {}
  virtual void init() final;
  void process(const Input&, const Output&) const;

private:
  const algorithms::UniqueIDGenSvc& m_uid = algorithms::UniqueIDGenSvc::instance();
};

} // namespace eicrecon
