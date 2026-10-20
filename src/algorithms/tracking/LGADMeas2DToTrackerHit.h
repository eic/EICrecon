// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Chun Yuen Tsang

#pragma once

#include <DDRec/CellIDPositionConverter.h>
#include <algorithms/algorithm.h>
#include <edm4eic/Measurement2DCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <memory>
#include <string>
#include <string_view>

#include "algorithms/tracking/ActsGeometryProvider.h"

namespace eicrecon {

using LGADMeas2DToTrackerHitAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::Measurement2DCollection>,
    algorithms::Output<edm4eic::TrackerHitCollection>>;

class LGADMeas2DToTrackerHit : public LGADMeas2DToTrackerHitAlgorithm {

public:
  LGADMeas2DToTrackerHit(std::string_view name)
      : LGADMeas2DToTrackerHitAlgorithm{
            name, {"TOFBarrelRecMeas2D"}, {"TOFBarrelRecHits"}, ""} {};

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  /// Cell ID position converter
  const dd4hep::rec::CellIDPositionConverter* m_converter = nullptr;

  std::shared_ptr<const ActsGeometryProvider> m_acts_context;
};

} // namespace eicrecon
