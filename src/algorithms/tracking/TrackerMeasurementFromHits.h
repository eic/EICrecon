// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Whitney Armstrong, Sylvester Joosten, Dmitry Romanov, Wouter Deconinck, Shujie Li

#pragma once

#include <DD4hep/Detector.h>
#include <DDRec/CellIDPositionConverter.h>
#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <edm4eic/Measurement2DCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <gsl/pointers>
#include <memory>
#include <string>
#include <string_view>

#include "algorithms/interfaces/ActsSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/tracking/ActsGeometryProvider.h"

namespace eicrecon {

using TrackerMeasurementFromHitsAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::TrackerHitCollection>,
                          algorithms::Output<edm4eic::Measurement2DCollection>>;

class TrackerMeasurementFromHits : public TrackerMeasurementFromHitsAlgorithm,
                                   public WithPodConfig<NoConfig> {
public:
  TrackerMeasurementFromHits(std::string_view name)
      : TrackerMeasurementFromHitsAlgorithm{name,
                                            {"inputTrackerHits"},
                                            {"outputMeasurements"},
                                            "convert tracker hits to measurements."} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  const algorithms::GeoSvc& m_geo{algorithms::GeoSvc::instance()};
  const dd4hep::Detector* m_dd4hepGeo{m_geo.detector()};
  const dd4hep::rec::CellIDPositionConverter* m_converter{m_geo.cellIDPositionConverter()};

  const algorithms::ActsSvc& m_acts{algorithms::ActsSvc::instance()};
  std::shared_ptr<const ActsGeometryProvider> m_acts_context{m_acts.acts_geometry_provider()};

  /// Detector-specific information
  unsigned long m_detid_b0tracker;
  unsigned long m_detid_OuterMPGD;
  static constexpr dd4hep::CellID m_pStripBit   = ((dd4hep::CellID)0x1) << 28;
  static constexpr dd4hep::CellID m_nStripBit   = ((dd4hep::CellID)0x2) << 28;

};

} // namespace eicrecon
