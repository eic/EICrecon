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

#include "LGADHitReconstructionConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using LGADHitReconstructionAlgorithm = 
	algorithms::Algorithm<algorithms::Input<edm4eic::RawTrackerHitCollection>,
	                      algorithms::Output<edm4eic::TrackerHitCollection>>;

class LGADHitReconstruction : public LGADHitReconstructionAlgorithm,
	                      public WithPodConfig<LGADHitReconstructionConfig> {

public:
  LGADHitReconstruction(std::string_view name) 
    : LGADHitReconstructionAlgorithm{name, {"TOFBarrelADCTDC"}, {"TOFBarrelRecHit"}, ""} {};

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  struct HitInfo {
    double x, y, z;
    int adc, tdc;
    dd4hep::rec::CellID id;
  };
  dd4hep::rec::CellID getSensorInfos(const dd4hep::rec::CellID& id) const;
  /** algorithm logger */
  std::shared_ptr<spdlog::logger> m_log;

  /// Cell ID position converter
  const dd4hep::rec::CellIDPositionConverter* m_converter = nullptr;

  /// fetch sensor information from cellID
  const dd4hep::DDSegmentation::BitFieldCoder* m_decoder = nullptr;

  const dd4hep::Detector* m_detector = nullptr;
};
} // namespace eicrecon
