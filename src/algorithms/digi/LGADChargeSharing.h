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
#include "algorithms/digi/LGADChargeSharingConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using LGADChargeSharingAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::SimTrackerHitCollection>,
                          algorithms::Output<edm4hep::SimTrackerHitCollection>>;

class LGADChargeSharing : public LGADChargeSharingAlgorithm,
                          public WithPodConfig<LGADChargeSharingConfig> {

public:
  LGADChargeSharing(std::string_view name)
      : LGADChargeSharingAlgorithm{name, {"TOFBarrelHits"}, {"TOFBarrelSharedHits"}, ""} {};

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  void _findAllNeighborsInSensor(dd4hep::rec::CellID hitCell,
                                 std::vector<dd4hep::rec::CellID>& answer,
                                 std::unordered_set<dd4hep::rec::CellID>& dp) const;
  double _integralGaus(double mean, double sd, double low_lim, double up_lim) const;
  dd4hep::Position _cell2LocalPosition(const dd4hep::rec::CellID& cell) const;
  dd4hep::Position _global2Local(const dd4hep::Position& pos) const;

  const dd4hep::DDSegmentation::BitFieldCoder* m_decoder  = nullptr;
  const dd4hep::Detector* m_detector                      = nullptr;
  const dd4hep::rec::CellIDPositionConverter* m_converter = nullptr;
  dd4hep::IDDescriptor m_idSpec;

  // helper function to find neighbors
  std::function<bool(const dd4hep::rec::CellID& id1, const dd4hep::rec::CellID& id2)>
      _is_same_sensor;
};

} // namespace eicrecon
