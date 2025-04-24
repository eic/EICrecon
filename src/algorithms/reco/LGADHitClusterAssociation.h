// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Chun Yuen Tsang

#pragma once

#include <DDRec/CellIDPositionConverter.h>
#include <DDSegmentation/BitFieldCoder.h>
#include <algorithms/algorithm.h>
#include <edm4eic/Measurement2DCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <spdlog/logger.h>
#include <memory>
#include <string>
#include <string_view>

#include "LGADHitClusterAssociationConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using LGADHitClusterAssociationAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::Measurement2DCollection, edm4eic::RawTrackerHitCollection>,
    algorithms::Output<edm4eic::TrackerHitCollection>>;

class LGADHitClusterAssociation : public LGADHitClusterAssociationAlgorithm,
                                  public WithPodConfig<LGADHitClusterAssociationConfig> {

public:
  LGADHitClusterAssociation(std::string_view name)
      : LGADHitClusterAssociationAlgorithm{
            name, {"TOFBarrelClusterHits", "TOFBarrelRawHits"}, {"TOFBarrelRecHits"}, ""} {};

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  dd4hep::rec::CellID getSensorInfos(const dd4hep::rec::CellID& id) const;

  std::shared_ptr<spdlog::logger> m_log;
  /// fetch sensor information from cellID
  const dd4hep::DDSegmentation::BitFieldCoder* m_decoder = nullptr;
};
} // namespace eicrecon
