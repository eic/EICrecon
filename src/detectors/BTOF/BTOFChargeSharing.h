// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Chun Yuen Tsang, Prithwish Tribedy
//
// Spread energy desposition from one strip to neighboring strips within sensor boundaries

// Author: Chun Yuen Tsang
// Date: 10/22/2024

#pragma once

#include "TF1.h"
#include <iostream>
#include <memory>
#include <string_view>
#include <random>
#include <vector>
#include <unordered_set>

#include <DD4hep/Detector.h>
#include <algorithms/algorithm.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <podio/ObjectID.h>
#include <spdlog/spdlog.h>

#include "DD4hep/Detector.h"
#include "DDRec/Surface.h"
#include <DDRec/CellIDPositionConverter.h>

#include "algorithms/digi/TOFHitDigiConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using BTOFChargeSharingAlgorithm =
        algorithms::Algorithm<algorithms::Input<edm4hep::SimTrackerHitCollection>,
                              algorithms::Output<edm4hep::SimTrackerHitCollection>>;

class BTOFChargeSharing : public BTOFChargeSharingAlgorithm,
                          public WithPodConfig<TOFHitDigiConfig> {

public:
  BTOFChargeSharing(std::string_view name) : BTOFChargeSharingAlgorithm{name,
                                                  {"TOFBarrelHits"},
                                                  {"TOFBarrelSharedHits"},
                                                  ""} {};

  void init() final;
  void process(const Input&, const Output&) const final;
protected:
  void _findAllNeighborsInSensor(dd4hep::rec::CellID hitCell,
                                 std::shared_ptr<std::vector<dd4hep::rec::CellID>>& ans,
                                 std::unordered_set<dd4hep::rec::CellID>& dp) const;
  const dd4hep::rec::CellID _getSensorID(const dd4hep::rec::CellID& hitCell) const;
  double _integralGaus(double mean, double sd, double low_lim, double up_lim) const;
  dd4hep::Position _cell2LocalPosition(const dd4hep::rec::CellID& cell) const;
  dd4hep::Position _global2Local(const dd4hep::Position& pos) const;


  // can't cache our results if process has to be const
  //bool m_useCache                                         = true;
  const dd4hep::DDSegmentation::BitFieldCoder* m_decoder  = nullptr;
  const dd4hep::Detector* m_detector                      = nullptr;
  const dd4hep::rec::CellIDPositionConverter* m_converter = nullptr;

  //std::unordered_map<dd4hep::rec::CellID, std::shared_ptr<std::vector<dd4hep::rec::CellID>>> m_cache;

};

} // namespace eicrecon
