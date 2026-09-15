// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ePIC Collaboration

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <functional>
#include <map>
#include <utility>

#include "algorithms/tracking/ActsToTracksTruthWeights.h"

using Catch::Approx;
using eicrecon::detail::accumulateMeasurementTruthWeights;

TEST_CASE("ActsToTracks truth weights count measurements, not SimTrackerHit records") {
  std::map<int, double> trackWeights;

  // One measurement contains ten association records from particle 1.
  accumulateMeasurementTruthWeights(std::map<int, double>{}, {{1, 10.0}}, trackWeights);

  // A second independent measurement contains one association from particle 2.
  accumulateMeasurementTruthWeights(std::map<int, double>{}, {{2, 1.0}}, trackWeights);

  // Each measurement contributes one unit in total, so the two particles are
  // tied before the final track-level normalization (0.5 / 0.5 afterwards).
  REQUIRE(trackWeights.at(1) == Approx(1.0));
  REQUIRE(trackWeights.at(2) == Approx(1.0));
}

TEST_CASE("ActsToTracks truth weights use energy fractions within a measurement") {
  std::map<int, double> trackWeights;

  accumulateMeasurementTruthWeights({{1, 12.0}, {2, 3.0}}, {{1, 2.0}, {2, 1.0}}, trackWeights);

  REQUIRE(trackWeights.at(1) == Approx(0.8));
  REQUIRE(trackWeights.at(2) == Approx(0.2));
  REQUIRE(trackWeights.at(1) + trackWeights.at(2) == Approx(1.0));
}

TEST_CASE("ActsToTracks truth weights fall back to association counts") {
  std::map<int, double> trackWeights;

  accumulateMeasurementTruthWeights({}, {{1, 2.0}, {2, 1.0}}, trackWeights);

  REQUIRE(trackWeights.at(1) == Approx(2.0 / 3.0));
  REQUIRE(trackWeights.at(2) == Approx(1.0 / 3.0));
}
