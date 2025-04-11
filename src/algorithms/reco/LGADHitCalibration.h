// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Chun Yuen Tsang

#pragma once

#include <DD4hep/Detector.h>
#include <DDRec/CellIDPositionConverter.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <algorithms/algorithm.h>
#include <memory>
#include <spdlog/logger.h>
#include <algorithms/geo.h>
#include <algorithms/service.h>

#include "LGADHitCalibrationConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using LGADHitCalibrationAlgorithm =
        algorithms::Algorithm<algorithms::Input<edm4eic::RawTrackerHitCollection>,
                              algorithms::Output<edm4eic::TrackerHitCollection>>;

class LGADHitCalibration : public LGADHitCalibrationAlgorithm,
                              public WithPodConfig<LGADHitCalibrationConfig> {

public:
  LGADHitCalibration(std::string_view name)
    : LGADHitCalibrationAlgorithm{name, {"TOFBarrelADCTDC"}, {"TOFBarrelCalHit"}, ""} {};

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  std::shared_ptr<spdlog::logger> m_log;

  const dd4hep::rec::CellIDPositionConverter* m_converter = nullptr;
};
} // namespace eicrecon
