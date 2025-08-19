// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Chun Yuen Tsang

#pragma once

#include <DD4hep/VolumeManager.h>
#include <DDRec/CellIDPositionConverter.h>
#include <DDSegmentation/BitFieldCoder.h>
#include <algorithms/algorithm.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <edm4hep/Vector2f.h>
#include <edm4hep/Vector3f.h>
#include <spdlog/logger.h>
#include <memory>
#include <string>
#include <string_view>

#include "LGADHitAssociationConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using LGADHitAssociationAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::TrackerHitCollection, edm4hep::SimTrackerHitCollection>,
    algorithms::Output<edm4eic::MCRecoTrackerHitAssociationCollection>>;

class LGADHitAssociation : public LGADHitAssociationAlgorithm,
                           public WithPodConfig<LGADHitAssociationConfig> {

public:
  LGADHitAssociation(std::string_view name)
      : LGADHitAssociationAlgorithm{
            name, {"TOFBarrelCalHits", "TOFBarrelRawHits"}, {"TOFBarrelAssoHits"}, ""} {};

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  dd4hep::rec::CellID getSensorInfos(const dd4hep::rec::CellID& id) const;
  edm4hep::Vector3f _local2Global(const dd4hep::VolumeManagerContext* context,
                                  const edm4hep::Vector2f& locPos) const;

  std::shared_ptr<spdlog::logger> m_log;
  /// fetch sensor information from cellID
  const dd4hep::DDSegmentation::BitFieldCoder* m_decoder  = nullptr;
  const dd4hep::rec::CellIDPositionConverter* m_converter = nullptr;
};
} // namespace eicrecon
