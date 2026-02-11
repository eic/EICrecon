// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#include <algorithms/logger.h>
#include <catch2/catch_test_macros.hpp>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <memory>
#include <string>

#include "algorithms/calorimetry/CalorimeterHitToTrackerHit.h"

using eicrecon::CalorimeterHitToTrackerHit;

TEST_CASE("the algorithm runs", "[CalorimeterHitToTrackerHit]") {

  SECTION("empty input produces empty output") {
    CalorimeterHitToTrackerHit algo("CalorimeterHitToTrackerHit");

    std::shared_ptr<spdlog::logger> logger =
        spdlog::default_logger()->clone("CalorimeterHitToTrackerHit");
    logger->set_level(spdlog::level::trace);

    algo.level(algorithms::LogLevel(spdlog::level::trace));
    algo.init();

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
}
