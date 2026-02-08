// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Wouter Deconinck

#pragma once

#include <DD4hep/Detector.h>
#include <DD4hep/VolumeManager.h>
#include <DDRec/CellIDPositionConverter.h>
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

/**
 * @brief Convert calorimeter hits to tracker hits for tracking reconstruction
 *
 * This algorithm converts calorimeter hits into tracker hits by determining
 * position uncertainties from the detector segmentation. Currently supports
 * CartesianGridXY segmentation.
 *
 * @note This algorithm requires DD4hep VolumeManager with properly configured
 * detector elements and cannot be easily unit tested with mock detectors.
 * Testing is performed through integration tests with complete detector geometry.
 */
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
  const dd4hep::rec::CellIDPositionConverter* m_converter{
      algorithms::GeoSvc::instance().cellIDPositionConverter()};
  const dd4hep::VolumeManager m_volume_manager{m_detector->volumeManager()};
};

} // namespace eicrecon
