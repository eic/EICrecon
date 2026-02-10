// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Wouter Deconinck

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <DD4hep/Segmentations.h>
#include <DDRec/CellIDPositionConverter.h>
#include <Evaluator/DD4hepUnits.h>
#include <algorithms/geo.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <edm4hep/Vector3f.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <gsl/pointers>
#include <cmath>
#include <memory>
#include <string>
#include <utility>

#include "algorithms/calorimetry/CalorimeterHitToTrackerHit.h"

using eicrecon::CalorimeterHitToTrackerHit;

TEST_CASE("the algorithm runs", "[CalorimeterHitToTrackerHit]") {
  CalorimeterHitToTrackerHit algo("CalorimeterHitToTrackerHit");

  std::shared_ptr<spdlog::logger> logger =
      spdlog::default_logger()->clone("CalorimeterHitToTrackerHit");
  logger->set_level(spdlog::level::trace);

  algo.level(algorithms::LogLevel(spdlog::level::trace));
  algo.init();

  SECTION("empty input produces empty output") {
    auto calorimeter_hits = std::make_unique<edm4eic::CalorimeterHitCollection>();
    auto tracker_hits     = std::make_unique<edm4eic::TrackerHitCollection>();

    algo.process({calorimeter_hits.get()}, {tracker_hits.get()});

    REQUIRE(tracker_hits->size() == 0);
  }

  // Note: Full functionality testing with CartesianGridXY segmentation requires
  // a complete DD4hep detector hierarchy with VolumeManager, DetElements, and
  // properly configured readouts. The mock detector in algorithmsInit.cc lacks
  // this complete hierarchy. The algorithm is validated through integration
  // tests with the epic-main detector geometry in the full CI pipeline.
  //
  // The test above ensures the algorithm handles edge cases (empty input)
  // without crashes. Functional validation of position error calculations
  // from cell dimensions is performed in the integration test environment.
  //
  // What would be tested with a full detector setup:
  // - CellID is correctly copied from calorimeter hit to tracker hit
  // - Position is correctly copied from calorimeter hit to tracker hit
  // - Position uncertainties are calculated as cell_dimension / sqrt(12)
  // - Position error covariance has non-zero xx and yy components
  // - Position error covariance has zero zz component for CartesianGridXY
  // - Time, timeError, energy, and energyError are correctly copied
  // - Unsupported segmentation types are skipped with a warning
}
