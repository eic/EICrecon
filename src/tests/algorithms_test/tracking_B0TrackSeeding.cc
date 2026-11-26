// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Tom Bleher

#include <Acts/Definitions/Algebra.hpp>
#include <Acts/Definitions/Units.hpp>
#include <Acts/MagneticField/ConstantBField.hpp>
#include <Acts/MagneticField/MagneticFieldContext.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <edm4eic/Cov6f.h>
#include <edm4eic/CovDiag3f.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/TrackSeedCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <edm4hep/Vector3f.h>
#include <podio/RelationRange.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <cmath>
#include <cstddef>
#include <initializer_list>

#include "algorithms/tracking/B0TrackSeeding.h"
#include "algorithms/tracking/B0TrackSeedingConfig.h"
#include "algorithms/tracking/TrackSeeding.h"

namespace {

const double kFieldStrength = 1.3 * Acts::UnitConstants::T;
// Off the vertical, as far off the midplane of the B0 dipole: a frame along any other direction
// would bend the tracks in the slope the Acts finders take as straight
const double kFieldTilt = 0.5; // rad
const Acts::Vector3 kFieldDirection(std::sin(kFieldTilt), std::cos(kFieldTilt), 0.);

/// Hits on the four B0 stations, on a helix in the field that starts at z = 5800 mm along z and
/// 10 mrad along the field, with 3 ns hit times
edm4eic::TrackerHitCollection helixHits(double q, double p) {
  const Acts::Vector3 across = Acts::Vector3::UnitZ().cross(kFieldDirection); // the force for q > 0
  const double radius        = p / kFieldStrength;
  edm4eic::TrackerHitCollection hits;
  for (const double z : {5850., 6272., 6601., 6880.}) {
    const double dz              = z - 5800.;
    const Acts::Vector3 position = q * (radius - std::sqrt(radius * radius - dz * dz)) * across +
                                   0.01 * dz * kFieldDirection + z * Acts::Vector3::UnitZ();
    hits.create(0ULL,
                edm4hep::Vector3f{static_cast<float>(position.x()),
                                  static_cast<float>(position.y()),
                                  static_cast<float>(position.z())},
                edm4eic::CovDiag3f{4.e-4F, 4.e-4F, 0.F}, 3.F, 0.F, 0.F, 0.F);
  }
  return hits;
}

} // namespace

// The test environment has no Acts magnetic field service, so the tests call findSeeds() with an
// Acts::ConstantBField instead of process().

namespace {

/// Seeds of the given hits with the given configuration
std::size_t countSeeds(const eicrecon::B0TrackSeedingConfig& cfg,
                       const edm4eic::TrackerHitCollection& hits) {
  const Acts::ConstantBField field(kFieldStrength * kFieldDirection);
  eicrecon::B0TrackSeeding algo("B0TrackSeeding");
  algo.applyConfig(cfg);
  algo.init();
  edm4eic::TrackSeedCollection seeds;
  edm4eic::TrackParametersCollection parameters;
  algo.findSeeds(hits, field, Acts::MagneticFieldContext{}, seeds, parameters);
  return seeds.size();
}

} // namespace

TEST_CASE("B0TrackSeeding finds helices in a field off the vertical", "[B0TrackSeeding]") {
  const Acts::ConstantBField field(kFieldStrength * kFieldDirection);
  const eicrecon::B0TrackSeedingConfig cfg;
  eicrecon::B0TrackSeeding algo("B0TrackSeeding");
  algo.applyConfig(cfg);
  algo.init();

  for (const double q : {+1., -1.}) {
    for (const double p : {5., 20., 100.}) {
      const auto hits = helixHits(q, p);
      edm4eic::TrackSeedCollection seeds;
      edm4eic::TrackParametersCollection parameters;
      algo.findSeeds(hits, field, Acts::MagneticFieldContext{}, seeds, parameters);

      // Stations 1-2-3, 1-2-4, 1-3-4 and 2-3-4
      REQUIRE(seeds.size() == 4);
      for (const auto seed : seeds) {
        const auto params = seed.getParams();
        CHECK(params.getQOverP() == Catch::Approx(q / p).epsilon(0.02));
        CHECK(params.getTime() == Catch::Approx(3.));
        CHECK(std::sqrt(params.getCovariance()(4, 4)) ==
              Catch::Approx(cfg.qOverPRelativeError / p).epsilon(0.02));
        // EDM4eic orders the covariance as phi, theta: near the z axis of the perigee, phi is the
        // less constrained
        CHECK(params.getCovariance()(2, 2) > params.getCovariance()(3, 3));

        // The perigee is upstream of the first hit by the anchor distance
        const auto first   = seed.getHits()[0].getPosition();
        const auto perigee = seed.getPerigee();
        CHECK(perigee.z < first.z);
        CHECK(std::hypot(first.x - perigee.x, first.y - perigee.y, first.z - perigee.z) ==
              Catch::Approx(cfg.anchorDistance).epsilon(0.01));
      }
    }
  }
}

TEST_CASE("B0TrackSeeding keeps stiff tracks with hits off the helix by their resolution",
          "[B0TrackSeeding]") {
  // At 250 GeV multiple scattering no longer opens the slope cut of the Acts triplet finder, so
  // the hit resolution alone has to cover a middle hit displaced by two standard deviations
  const auto exact = helixHits(1., 250.);
  edm4eic::TrackerHitCollection hits;
  for (std::size_t i = 0; i < 3; ++i) {
    auto position = exact[i].getPosition();
    if (i == 1) {
      const Acts::Vector3 shift = 2. * 0.02 * kFieldDirection;
      position.x += static_cast<float>(shift.x());
      position.y += static_cast<float>(shift.y());
    }
    hits.create(0ULL, position, exact[i].getPositionError(), 3.F, 0.F, 0.F, 0.F);
  }

  eicrecon::B0TrackSeedingConfig cfg;
  CHECK(countSeeds(cfg, hits) == 1);
  // Unscaled, the hit errors open the slope cut by one standard deviation only
  cfg.hitErrorScale = 1.F;
  CHECK(countSeeds(cfg, hits) == 0);
}

TEST_CASE("B0TrackSeeding seeds tracks above minPt only", "[B0TrackSeeding]") {
  const auto hits = helixHits(1., 0.5);
  eicrecon::B0TrackSeedingConfig cfg;
  CHECK(countSeeds(cfg, hits) == 0);
  cfg.minPt = static_cast<float>(0.1 * Acts::UnitConstants::GeV);
  CHECK(countSeeds(cfg, hits) == 4);
}

TEST_CASE("B0TrackSeeding makes no seed", "[B0TrackSeeding]") {
  eicrecon::B0TrackSeeding algo("B0TrackSeeding");
  algo.applyConfig(eicrecon::B0TrackSeedingConfig{});
  algo.init();
  edm4eic::TrackSeedCollection seeds;
  edm4eic::TrackParametersCollection parameters;

  SECTION("without a field") {
    const Acts::ConstantBField field(Acts::Vector3::Zero());
    algo.findSeeds(helixHits(1., 20.), field, Acts::MagneticFieldContext{}, seeds, parameters);
  }
  SECTION("without hits") {
    const Acts::ConstantBField field(kFieldStrength * kFieldDirection);
    algo.findSeeds(edm4eic::TrackerHitCollection{}, field, Acts::MagneticFieldContext{}, seeds,
                   parameters);
  }

  CHECK(seeds.empty());
  CHECK(parameters.empty());
}
