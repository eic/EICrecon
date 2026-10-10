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

#include "algorithms/tracking/CKFTracking.h"

using eicrecon::timeSystemIDsForReadouts;

// The mock geometry (see algorithmsInit.cc) has a DetElement "MockMPGD" with system ID 3
// and a sensitive detector of the same name with readout "MockMPGDHits".

TEST_CASE("CKFTracking timeSystemIDsForReadouts: readout resolves to the system ID of its detector",
          "[CKFTracking]") {
  const dd4hep::Detector& detector = *algorithms::GeoSvc::instance().detector();
  CHECK(timeSystemIDsForReadouts(detector, {"MockMPGDHits"}) == std::set<std::uint8_t>{3});
}

TEST_CASE("CKFTracking timeSystemIDsForReadouts: empty list resolves to no systems",
          "[CKFTracking]") {
  const dd4hep::Detector& detector = *algorithms::GeoSvc::instance().detector();
  CHECK(timeSystemIDsForReadouts(detector, {}).empty());
}

TEST_CASE("CKFTracking timeSystemIDsForReadouts: unknown readout throws", "[CKFTracking]") {
  const dd4hep::Detector& detector = *algorithms::GeoSvc::instance().detector();
  CHECK_THROWS_AS(timeSystemIDsForReadouts(detector, {"NoSuchHits"}), std::runtime_error);
  // A readout without a sensitive detector (no DetElement) also throws
  CHECK_THROWS_AS(timeSystemIDsForReadouts(detector, {"MockTrackerHits"}), std::runtime_error);
}
