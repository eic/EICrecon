// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Chun Yuen Tsang

#pragma once

#include <DD4hep/Detector.h>
#include <DDRec/CellIDPositionConverter.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <memory>
#include <spdlog/logger.h>

#include "BTOFHitReconstructionConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

/**
 * Produces edm4eic::TrackerHit with geometric info from edm4eic::RawTrackerHit
 */
class BTOFHitReconstruction : public WithPodConfig<BTOFHitReconstructionConfig> {

public:
  /// Once in a lifetime initialization
  void init(const dd4hep::rec::CellIDPositionConverter* converter, const dd4hep::Detector* detector,
            std::shared_ptr<spdlog::logger>& logger);

  /// Processes RawTrackerHit and produces a TrackerHit
  std::unique_ptr<edm4eic::TrackerHitCollection>
  process(const edm4eic::RawTrackerHitCollection& TDCADC_hits);

private:
  struct HitInfo {
    double x, y, z;
    int adc, tdc;
    dd4hep::rec::CellID id;
  };
  dd4hep::rec::CellID getDetInfos(const dd4hep::rec::CellID& id);
  /** algorithm logger */
  std::shared_ptr<spdlog::logger> m_log;

  /// Cell ID position converter
  const dd4hep::rec::CellIDPositionConverter* m_converter = nullptr;

  /// fetch sensor information from cellID
  const dd4hep::DDSegmentation::BitFieldCoder* m_decoder = nullptr;

  const dd4hep::Detector* m_detector = nullptr;
};
} // namespace eicrecon
