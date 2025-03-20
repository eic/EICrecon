// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Chun Yuen Tsang, Prithwish Tribedy
//
// Spread energy deposition from one strip to neighboring strips within sensor boundaries

#pragma once

#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Objects.h>
#include <DDRec/CellIDPositionConverter.h>
#include <DDSegmentation/BitFieldCoder.h>
#include <algorithms/algorithm.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "DD4hep/Detector.h"
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
      : SiliconChargeSharingAlgorithm{name, {"TOFBarrelHits"}, {"TOFBarrelSharedHits"}, ""} {};

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  void findAllNeighborsInSensor(dd4hep::rec::CellID test_CellID,
                                std::unordered_set<dd4hep::rec::CellID>& tested_cells,
                                std::vector<std::pair<dd4hep::rec::CellID, double>>& cell_charge,
                                double edep, dd4hep::Position hitPos) const;
  double energyAtCell(const dd4hep::rec::CellID& cell, dd4hep::Position hitPos, double edep) const;
  double integralGaus(double mean, double sd, double low_lim, double up_lim) const;
  dd4hep::Position cell2LocalPosition(const dd4hep::rec::CellID& cell) const;
  dd4hep::Position global2Local(const edm4hep::SimTrackerHit& hit) const;

  const dd4hep::Detector*                      m_detector  = nullptr;
  const dd4hep::rec::CellIDPositionConverter*  m_converter = nullptr;
  dd4hep::Segmentation                   m_seg;

};

} // namespace eicrecon
