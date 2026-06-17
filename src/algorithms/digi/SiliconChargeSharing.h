// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024-2025 Chun Yuen Tsang, Prithwish Tribedy, Simon Gardner
//
// Spread energy deposition from one strip to neighboring strips within sensor boundaries

#pragma once

#include <DD4hep/DetElement.h>
#include <DD4hep/Objects.h>
#include <DD4hep/Segmentations.h>
#include <DDRec/CellIDPositionConverter.h>
#include <DDSegmentation/CartesianGridXY.h>
#include <TGeoMatrix.h>
#include <algorithms/algorithm.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "algorithms/digi/SiliconChargeSharingConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using SiliconChargeSharingAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::SimTrackerHitCollection>,
                          algorithms::Output<edm4hep::SimTrackerHitCollection>>;

class SiliconChargeSharing : public SiliconChargeSharingAlgorithm,
                             public WithPodConfig<SiliconChargeSharingConfig> {

public:
  SiliconChargeSharing(std::string_view name)
      : SiliconChargeSharingAlgorithm{name, {"inputHits"}, {"outputSharedHits"}, ""} {};

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  void findAllNeighborsInSensor(const dd4hep::rec::CellID testCellID,
                                std::unordered_set<dd4hep::rec::CellID>& tested_cells,
                                const float edep, const dd4hep::Position hitPos,
                                const dd4hep::DDSegmentation::CartesianGridXY* segmentation,
                                const std::pair<double, double>& xy_range,
                                const edm4hep::SimTrackerHit& hit,
                                edm4hep::SimTrackerHitCollection* sharedHits) const;
  float energyAtCell(const double xDimension, const double yDimension,
                     const dd4hep::Position localPos, const dd4hep::Position hitPos,
                     const float edep) const;
  static float integralGaus(float mean, float sd, float low_lim, float up_lim);
  dd4hep::Position cell2LocalPosition(const dd4hep::rec::CellID& cell) const;
  static dd4hep::Position global2Local(const dd4hep::Position& globalPosition,
                                       const TGeoHMatrix* transform);
  const dd4hep::DDSegmentation::CartesianGridXY*
  getLocalSegmentation(const dd4hep::rec::CellID& cellID) const;

  mutable std::unordered_map<const dd4hep::DetElement*, const TGeoHMatrix*> m_transform_map;
  mutable std::unordered_map<const dd4hep::DetElement*,
                             const dd4hep::DDSegmentation::CartesianGridXY*>
      m_segmentation_map;
  mutable std::unordered_map<const dd4hep::DetElement*, const std::pair<double, double>>
      m_xy_range_map;
  const dd4hep::rec::CellIDPositionConverter* m_converter = nullptr;
  dd4hep::Segmentation m_seg;
};

} // namespace eicrecon
