// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
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

  SECTION("single calorimeter hit with CartesianGridXY segmentation") {
    auto detector = algorithms::GeoSvc::instance().detector();

    // Use pre-configured test calorimeter readout from algorithmsInit.cc
    auto id_desc = detector->readout("TestCalorimeterReadout").idSpec();

    // Create algorithm after detector is already set up
    CalorimeterHitToTrackerHit algo("CalorimeterHitToTrackerHit");

    std::shared_ptr<spdlog::logger> logger =
        spdlog::default_logger()->clone("CalorimeterHitToTrackerHit");
    logger->set_level(spdlog::level::trace);

    algo.level(algorithms::LogLevel(spdlog::level::trace));
    algo.init();

    // Create a calorimeter hit
    auto calorimeter_hits = std::make_unique<edm4eic::CalorimeterHitCollection>();
    auto tracker_hits     = std::make_unique<edm4eic::TrackerHitCollection>();

    // Encode cellID with system=100, x=5, y=7
    auto cell_id = id_desc.encode({{"system", 100}, {"x", 5}, {"y", 7}});

    calorimeter_hits->create(cell_id,                            // cellID
                             1.5,                                // energy (GeV)
                             0.05,                               // energyError
                             10.0,                               // time (ns)
                             0.5,                                // timeError
                             edm4hep::Vector3f(10.0, 21.0, 0.0), // position (mm)
                             edm4hep::Vector3f(2.0, 3.0, 10.0),  // dimension
                             0,                                  // sector
                             0,                                  // layer
                             edm4hep::Vector3f(10.0, 21.0, 0.0)  // local position
    );

    algo.process({calorimeter_hits.get()}, {tracker_hits.get()});

    REQUIRE(tracker_hits->size() == 1);

    auto& tracker_hit = (*tracker_hits)[0];

    // Check that cellID is copied correctly
    REQUIRE(tracker_hit.getCellID() == cell_id);

    // Check that position is copied correctly
    REQUIRE_THAT(tracker_hit.getPosition().x, Catch::Matchers::WithinAbs(10.0, 1e-5));
    REQUIRE_THAT(tracker_hit.getPosition().y, Catch::Matchers::WithinAbs(21.0, 1e-5));
    REQUIRE_THAT(tracker_hit.getPosition().z, Catch::Matchers::WithinAbs(0.0, 1e-5));

    // Check that time and energy are copied correctly
    REQUIRE_THAT(tracker_hit.getTime(), Catch::Matchers::WithinAbs(10.0, 1e-5));
    REQUIRE_THAT(tracker_hit.getTimeError(), Catch::Matchers::WithinAbs(0.5, 1e-5));
    REQUIRE_THAT(tracker_hit.getEdep(), Catch::Matchers::WithinAbs(1.5, 1e-5));
    REQUIRE_THAT(tracker_hit.getEdepError(), Catch::Matchers::WithinAbs(0.05, 1e-5));

    // Check position uncertainties: sigma = dimension / sqrt(12)
    // For 2mm x-cell: sigma_x = 2.0 / sqrt(12) = 0.577... mm
    // For 3mm y-cell: sigma_y = 3.0 / sqrt(12) = 0.866... mm
    // Covariance = sigma^2
    const double expected_cov_xx = (2.0 / std::sqrt(12.0)) * (2.0 / std::sqrt(12.0));
    const double expected_cov_yy = (3.0 / std::sqrt(12.0)) * (3.0 / std::sqrt(12.0));

    auto cov = tracker_hit.getCovariance();
    REQUIRE_THAT(cov.xx, Catch::Matchers::WithinAbs(expected_cov_xx, 1e-5));
    REQUIRE_THAT(cov.yy, Catch::Matchers::WithinAbs(expected_cov_yy, 1e-5));
    REQUIRE_THAT(cov.zz, Catch::Matchers::WithinAbs(0.0, 1e-5));
  }
}
