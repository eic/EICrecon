
// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Sylvester Joosten, Wouter Deconinck, Chao, Whitney Armstrong

// Reconstruct digitized outputs, paired with Jug::Digi::CalorimeterHitDigi
// Author: Chao Peng
// Date: 06/14/2021

#pragma once

#include <DD4hep/Detector.h>
#include <DD4hep/VolumeManager.h>
#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <gsl/pointers>
#include <string>
#include <string_view>

namespace eicrecon {

using CalorimeterHitToTrackerHitAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::CalorimeterHitCollection>,
                          algorithms::Output<edm4eic::TrackerHitCollection>>;

class CalorimeterHitToTrackerHit : public CalorimeterHitToTrackerHitAlgorithm {

public:
  CalorimeterHitToTrackerHit(std::string_view name)
      : CalorimeterHitToTrackerHitAlgorithm{name,
                                            {"inputCalorimeterHitCollection"},
                                            {"outputTrackerHitCollection"},
                                            "Convert calorimeter hits into tracker hits."} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  const dd4hep::Detector* m_detector{algorithms::GeoSvc::instance().detector()};
  const dd4hep::VolumeManager m_volume_manager{m_detector->volumeManager()};
};

} // namespace eicrecon
