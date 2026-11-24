// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#include <DD4hep/Detector.h>
#include <algorithms/geo.h>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <gsl/pointers>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "algorithms/tracking/ActsGeometryProvider.h"

// The mock geometry (see algorithmsInit.cc) has a DetElement "MockMPGD" with system ID 3
// and a sensitive detector of the same name with readout "MockMPGDHits".

TEST_CASE("ActsGeometryProvider geometryIdExtra: low 8 bits of the DD4hep identifier",
          "[ActsGeometryProvider]") {
  CHECK(ActsGeometryProvider::geometryIdExtra(3) == 3);
  CHECK(ActsGeometryProvider::geometryIdExtra(0x1234) == 0x34);
}

TEST_CASE("ActsGeometryProvider geometryIdExtrasForReadouts: readout resolves to the system ID "
          "of its detector",
          "[ActsGeometryProvider]") {
  const dd4hep::Detector& detector = *algorithms::GeoSvc::instance().detector();
  CHECK(ActsGeometryProvider::geometryIdExtrasForReadouts(detector, {"MockMPGDHits"}) ==
        std::set<std::uint8_t>{3});
}

TEST_CASE("ActsGeometryProvider geometryIdExtrasForReadouts: empty list resolves to no systems",
          "[ActsGeometryProvider]") {
  const dd4hep::Detector& detector = *algorithms::GeoSvc::instance().detector();
  CHECK(ActsGeometryProvider::geometryIdExtrasForReadouts(detector, {}).empty());
}

TEST_CASE("ActsGeometryProvider geometryIdExtrasForReadouts: unknown readout throws",
          "[ActsGeometryProvider]") {
  const dd4hep::Detector& detector = *algorithms::GeoSvc::instance().detector();
  CHECK_THROWS_AS(ActsGeometryProvider::geometryIdExtrasForReadouts(detector, {"NoSuchHits"}),
                  std::runtime_error);
  // A readout without a sensitive detector (no DetElement) also throws
  CHECK_THROWS_AS(ActsGeometryProvider::geometryIdExtrasForReadouts(detector, {"MockTrackerHits"}),
                  std::runtime_error);
}
