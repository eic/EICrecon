// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Chun Yuen Tsang

#pragma once

#include <DD4hep/Detector.h>
#include <DD4hep/Segmentations.h>
#include <DDRec/CellIDPositionConverter.h>
#include <DDSegmentation/BitFieldCoder.h>
#include <algorithms/algorithm.h>
#include <edm4eic/Measurement2DCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <spdlog/logger.h>
#include <memory>
#include <string>
#include <string_view>

#include "LGADHitClusteringConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using LGADHitClusteringAlgorithm =
        algorithms::Algorithm<algorithms::Input<edm4eic::TrackerHitCollection>,
                              algorithms::Output<edm4eic::Measurement2DCollection>>;

class LGADHitClustering : public LGADHitClusteringAlgorithm,
                              public WithPodConfig<LGADHitClusteringConfig> {

public:
  LGADHitClustering(std::string_view name)
    : LGADHitClusteringAlgorithm{name, {"TOFBarrelCalHit"}, {"TOFBarrelRecHit"}, ""} {};

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  dd4hep::rec::CellID getSensorInfos(const dd4hep::rec::CellID& id) const;
  /** algorithm logger */
  std::shared_ptr<spdlog::logger> m_log;

  /// Cell ID position converter
  const dd4hep::rec::CellIDPositionConverter* m_converter = nullptr;

  /// fetch sensor information from cellID
  const dd4hep::DDSegmentation::BitFieldCoder* m_decoder = nullptr;

  const dd4hep::Detector* m_detector = nullptr;

  dd4hep::Segmentation m_seg;
};
} // namespace eicrecon
