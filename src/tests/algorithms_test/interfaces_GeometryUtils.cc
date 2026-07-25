// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Segmentations.h>
#include <algorithms/geo.h>
#include <catch2/catch_test_macros.hpp>
#include <gsl/pointers>
#include <optional>
#include <string>

#include "algorithms/interfaces/GeometryUtils.h"

// The mock detector is set up by the global algorithmsInitListener (algorithmsInit.cc) and
// exposes the readouts "MockCalorimeterHits", "MockTrackerHits", "MockSiliconHits" and
// "MockMPGDHits". "MockTrackerHits", "MockSiliconHits" and "MockMPGDHits" carry segmentations.

TEST_CASE("hasReadout reports presence without throwing", "[GeometryUtils]") {
  const dd4hep::Detector* detector = algorithms::GeoSvc::instance().detector();

  SECTION("present readouts are found") {
    REQUIRE(eicrecon::geo::hasReadout(*detector, "MockCalorimeterHits"));
    REQUIRE(eicrecon::geo::hasReadout(*detector, "MockTrackerHits"));
  }

  SECTION("absent readout is reported as missing, not by throwing") {
    REQUIRE_NOTHROW(eicrecon::geo::hasReadout(*detector, "NonexistentHits"));
    REQUIRE_FALSE(eicrecon::geo::hasReadout(*detector, "NonexistentHits"));
  }
}

TEST_CASE("readoutIdSpec returns the descriptor or nullopt", "[GeometryUtils]") {
  const dd4hep::Detector* detector = algorithms::GeoSvc::instance().detector();

  SECTION("present readout yields a valid descriptor") {
    const auto id_spec = eicrecon::geo::readoutIdSpec(*detector, "MockCalorimeterHits");
    REQUIRE(id_spec.has_value());
    // "system:8,layer:8,x:8,y:8" -> four fields
    REQUIRE(id_spec->fields().size() == 4);
  }

  SECTION("absent readout yields nullopt without throwing") {
    REQUIRE_NOTHROW(eicrecon::geo::readoutIdSpec(*detector, "NonexistentHits"));
    REQUIRE_FALSE(eicrecon::geo::readoutIdSpec(*detector, "NonexistentHits").has_value());
  }
}

TEST_CASE("readoutSegmentation returns the segmentation or nullopt", "[GeometryUtils]") {
  const dd4hep::Detector* detector = algorithms::GeoSvc::instance().detector();

  SECTION("present readout with a segmentation yields a valid segmentation") {
    const auto segmentation = eicrecon::geo::readoutSegmentation(*detector, "MockTrackerHits");
    REQUIRE(segmentation.has_value());
    REQUIRE(segmentation->isValid());
  }

  SECTION("absent readout yields nullopt without throwing") {
    REQUIRE_NOTHROW(eicrecon::geo::readoutSegmentation(*detector, "NonexistentHits"));
    REQUIRE_FALSE(eicrecon::geo::readoutSegmentation(*detector, "NonexistentHits").has_value());
  }
}

TEST_CASE("helpers are non-throwing where raw DD4hep lookup would throw", "[GeometryUtils]") {
  const dd4hep::Detector* detector = algorithms::GeoSvc::instance().detector();

  // Raw DD4hep lookup of an absent readout throws (this is the behavior these helpers shield):
  REQUIRE_THROWS(detector->readout("NonexistentHits"));

  // The helpers report absence instead of throwing:
  REQUIRE_NOTHROW(eicrecon::geo::hasReadout(*detector, "NonexistentHits"));
  REQUIRE_NOTHROW(eicrecon::geo::readoutIdSpec(*detector, "NonexistentHits"));
  REQUIRE_NOTHROW(eicrecon::geo::readoutSegmentation(*detector, "NonexistentHits"));
}
