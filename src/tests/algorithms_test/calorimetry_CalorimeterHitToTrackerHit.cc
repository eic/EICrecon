// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025, Wouter Deconinck

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <DD4hep/Segmentations.h>
#include <Evaluator/DD4hepUnits.h>
#include <algorithms/geo.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <edm4hep/Vector3f.h>
#include <cmath>
#include <memory>
#include <string>

#include "algorithms/calorimetry/CalorimeterHitToTrackerHit.h"

using eicrecon::CalorimeterHitToTrackerHit;

TEST_CASE("the CalorimeterHitToTrackerHit algorithm runs", "[CalorimeterHitToTrackerHit]") {
  const float EPSILON = 1e-5;

  CalorimeterHitToTrackerHit algo("CalorimeterHitToTrackerHit");
  algo.init();

  auto detector = algorithms::GeoSvc::instance().detector();

  SECTION("converts calorimeter hits to tracker hits with CartesianGridXY") {
    // Use MockTrackerHits which has CartesianGridXY segmentation
    auto id_desc = detector->readout("MockTrackerHits").idSpec();

    auto calo_hits = std::make_unique<edm4eic::CalorimeterHitCollection>();
    auto tracker_hits = std::make_unique<edm4eic::TrackerHitCollection>();

    // Create a calorimeter hit
    auto calo_hit = calo_hits->create();
    calo_hit.setCellID(id_desc.encode({{"system", 2}, {"layer", 0}, {"x", 5}, {"y", 10}}));
    calo_hit.setEnergy(1.5 /* GeV */);
    calo_hit.setEnergyError(0.1 /* GeV */);
    calo_hit.setTime(10.0 /* ns */);
    calo_hit.setTimeError(0.5 /* ns */);
    calo_hit.setPosition(edm4hep::Vector3f{5.0, 10.0, 100.0});

    auto input = std::make_tuple(calo_hits.get());
    auto output = std::make_tuple(tracker_hits.get());

    algo.process(input, output);

    // Verify that one tracker hit was created
    REQUIRE(tracker_hits->size() == 1);

    auto tracker_hit = (*tracker_hits)[0];

    // Verify cellID is preserved
    REQUIRE(tracker_hit.getCellID() == calo_hit.getCellID());

    // Verify position is preserved
    REQUIRE_THAT(tracker_hit.getPosition().x, Catch::Matchers::WithinAbs(calo_hit.getPosition().x, EPSILON));
    REQUIRE_THAT(tracker_hit.getPosition().y, Catch::Matchers::WithinAbs(calo_hit.getPosition().y, EPSILON));
    REQUIRE_THAT(tracker_hit.getPosition().z, Catch::Matchers::WithinAbs(calo_hit.getPosition().z, EPSILON));

    // Verify energy is preserved
    REQUIRE_THAT(tracker_hit.getEdep(), Catch::Matchers::WithinAbs(calo_hit.getEnergy(), EPSILON));
    REQUIRE_THAT(tracker_hit.getEdepError(), Catch::Matchers::WithinAbs(calo_hit.getEnergyError(), EPSILON));

    // Verify time is preserved
    REQUIRE_THAT(tracker_hit.getTime(), Catch::Matchers::WithinAbs(calo_hit.getTime(), EPSILON));
    REQUIRE_THAT(tracker_hit.getTimeError(), Catch::Matchers::WithinAbs(calo_hit.getTimeError(), EPSILON));

    // Verify position errors are set properly for CartesianGridXY
    // Position error should be cell_dimension / sqrt(12)
    auto covariance = tracker_hit.getCovarianceMatrix();
    // For CartesianGridXY, xx and yy should be non-zero, zz should be 0
    REQUIRE(covariance.xx > 0);
    REQUIRE(covariance.yy > 0);
    REQUIRE_THAT(covariance.zz, Catch::Matchers::WithinAbs(0.0f, EPSILON));
  }

  SECTION("handles multiple calorimeter hits") {
    auto id_desc = detector->readout("MockTrackerHits").idSpec();

    auto calo_hits = std::make_unique<edm4eic::CalorimeterHitCollection>();
    auto tracker_hits = std::make_unique<edm4eic::TrackerHitCollection>();

    // Create multiple calorimeter hits
    for (int i = 0; i < 5; ++i) {
      auto calo_hit = calo_hits->create();
      calo_hit.setCellID(id_desc.encode({{"system", 2}, {"layer", 0}, {"x", i}, {"y", i}}));
      calo_hit.setEnergy(1.0 + i * 0.5 /* GeV */);
      calo_hit.setEnergyError(0.1 /* GeV */);
      calo_hit.setTime(10.0 /* ns */);
      calo_hit.setTimeError(0.5 /* ns */);
      calo_hit.setPosition(edm4hep::Vector3f{static_cast<float>(i), static_cast<float>(i), 100.0f});
    }

    auto input = std::make_tuple(calo_hits.get());
    auto output = std::make_tuple(tracker_hits.get());

    algo.process(input, output);

    // Verify that all hits were converted
    REQUIRE(tracker_hits->size() == 5);

    for (size_t i = 0; i < tracker_hits->size(); ++i) {
      auto tracker_hit = (*tracker_hits)[i];
      auto calo_hit = (*calo_hits)[i];

      REQUIRE(tracker_hit.getCellID() == calo_hit.getCellID());
      REQUIRE_THAT(tracker_hit.getEdep(), Catch::Matchers::WithinAbs(calo_hit.getEnergy(), EPSILON));
    }
  }

  SECTION("skips hits with unsupported segmentation type") {
    // Use MockCalorimeterHits which doesn't have a segmentation defined
    auto id_desc = detector->readout("MockCalorimeterHits").idSpec();

    auto calo_hits = std::make_unique<edm4eic::CalorimeterHitCollection>();
    auto tracker_hits = std::make_unique<edm4eic::TrackerHitCollection>();

    // Create a calorimeter hit with unsupported segmentation
    auto calo_hit = calo_hits->create();
    calo_hit.setCellID(id_desc.encode({{"system", 255}, {"layer", 0}, {"x", 0}, {"y", 0}}));
    calo_hit.setEnergy(1.5 /* GeV */);
    calo_hit.setEnergyError(0.1 /* GeV */);
    calo_hit.setTime(10.0 /* ns */);
    calo_hit.setTimeError(0.5 /* ns */);
    calo_hit.setPosition(edm4hep::Vector3f{0.0, 0.0, 100.0});

    auto input = std::make_tuple(calo_hits.get());
    auto output = std::make_tuple(tracker_hits.get());

    algo.process(input, output);

    // Verify that no tracker hits were created (unsupported segmentation is skipped)
    REQUIRE(tracker_hits->size() == 0);
  }

  SECTION("caches position errors for same detector element") {
    auto id_desc = detector->readout("MockTrackerHits").idSpec();

    auto calo_hits = std::make_unique<edm4eic::CalorimeterHitCollection>();
    auto tracker_hits = std::make_unique<edm4eic::TrackerHitCollection>();

    // Create multiple hits from the same detector element (same system, layer)
    // but different x, y coordinates
    for (int i = 0; i < 3; ++i) {
      auto calo_hit = calo_hits->create();
      calo_hit.setCellID(id_desc.encode({{"system", 2}, {"layer", 0}, {"x", i}, {"y", i}}));
      calo_hit.setEnergy(1.0 /* GeV */);
      calo_hit.setEnergyError(0.1 /* GeV */);
      calo_hit.setTime(10.0 /* ns */);
      calo_hit.setTimeError(0.5 /* ns */);
      calo_hit.setPosition(edm4hep::Vector3f{static_cast<float>(i), static_cast<float>(i), 100.0f});
    }

    auto input = std::make_tuple(calo_hits.get());
    auto output = std::make_tuple(tracker_hits.get());

    algo.process(input, output);

    // Verify all hits were converted
    REQUIRE(tracker_hits->size() == 3);

    // Verify that position errors are consistent across hits from same detector element
    auto first_cov = (*tracker_hits)[0].getCovarianceMatrix();
    for (size_t i = 1; i < tracker_hits->size(); ++i) {
      auto cov = (*tracker_hits)[i].getCovarianceMatrix();
      REQUIRE_THAT(cov.xx, Catch::Matchers::WithinAbs(first_cov.xx, EPSILON));
      REQUIRE_THAT(cov.yy, Catch::Matchers::WithinAbs(first_cov.yy, EPSILON));
      REQUIRE_THAT(cov.zz, Catch::Matchers::WithinAbs(first_cov.zz, EPSILON));
    }
  }
}
