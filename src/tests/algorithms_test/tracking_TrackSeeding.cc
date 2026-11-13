// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ePIC Collaboration

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <edm4eic/CovDiag3f.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/TrackSeedCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <edm4hep/Vector3f.h>
#include <podio/RelationRange.h>
#include <cmath>
#include <cstddef>
#include <string_view>

#include "algorithms/tracking/TrackSeeding.h"
#include "algorithms/tracking/TrackSeedingConfig.h"

using eicrecon::TrackSeeding;
using eicrecon::TrackSeedingConfig;

namespace {

/// Configuration that accepts the toy three-hit triplets used below with both back-ends.
TrackSeedingConfig threeHitConfig(TrackSeedingConfig::SeedingMethod seedingMethod) {
  TrackSeedingConfig cfg;
  cfg.seedingMethod = seedingMethod;
  cfg.rMin          = 0.0F;
  cfg.rMax          = 1000.0F;
  cfg.zMin          = -1000.0F;
  cfg.zMax          = 1000.0F;
  cfg.minPt         = 0.01F;
  // Use a small positive per-side Δr_min so that the middle SP is not counted
  // as its own top/bottom candidate. Acts' KD-tree range check is
  // min <= v < max (inclusive min), and DoubletSeedFinder accepts Δr == 0;
  // combined, Δr_min = 0 makes each hit appear as one of its own top
  // candidates and yields spurious self-referential seeds in Seeding2.
  // (cfg.deltaRMin only feeds Seeding2's BroadTripletSeedFilter, not the
  // doublet formation, so it does not need to be adjusted here.)
  cfg.deltaRMinBottomSP = 1.0F;
  cfg.deltaRMinTopSP    = 1.0F;
  cfg.deltaRMaxBottomSP = 1000.0F;
  cfg.deltaRMaxTopSP    = 1000.0F;
  // Loosen the two cuts that this toy triplet violates under default
  // configuration (both back-ends): the mid-top pair has Δφ ≈ 0.106 rad
  // (default cap 0.085), and the geometric impact parameter is ≈ 5 mm
  // (default cap 3 mm). All other defaults (rMinMiddle=20/rMaxMiddle=400,
  // cotThetaMax≈27, collisionRegion±250) are already satisfied.
  cfg.deltaPhiMax = 1.0F;
  cfg.impactMax   = 1000.0F;
  return cfg;
}

/// Three hits whose (r, z) are collinear up to the z of the outermost hit.
/// With z3 = 28.868 the r-z segment angles agree to ~0.2 mrad; moving z3
/// changes the middle->top polar angle while leaving the x-y circle untouched.
edm4eic::TrackerHitCollection threeHits(float z3) {
  edm4eic::TrackerHitCollection hits;
  hits.create(1, edm4hep::Vector3f(6.0F, 33.0F, 10.0F), edm4eic::CovDiag3f(), 0.0F, 0.0F, 1.0F,
              0.0F);
  hits.create(2, edm4hep::Vector3f(14.0F, 52.0F, 20.0F), edm4eic::CovDiag3f(), 0.0F, 0.0F, 1.0F,
              0.0F);
  hits.create(3, edm4hep::Vector3f(26.0F, 67.0F, z3), edm4eic::CovDiag3f(), 0.0F, 0.0F, 1.0F, 0.0F);
  return hits;
}

void checkThreeHitsProduceOneSeed(TrackSeedingConfig::SeedingMethod seedingMethod,
                                  std::string_view algoName) {
  TrackSeeding algo(algoName);
  algo.applyConfig(threeHitConfig(seedingMethod));
  algo.init();

  // Hits are chosen so that (r, z) is very nearly collinear. This is required
  // by Acts::SeedFinderOrthogonal, which has a hard-coded (non-configurable)
  // triplet alignment cut in the r-z plane of 0.005 rad, and by the default
  // deltaThetaMax = 0.005 rad that TrackSeeding applies in the Seeding2 path.
  const edm4eic::TrackerHitCollection hits = threeHits(28.868F);

  edm4eic::TrackSeedCollection seeds;
  edm4eic::TrackParametersCollection params;
  algo.process({&hits}, {&seeds, &params});

  REQUIRE(seeds.size() == 1);
  REQUIRE(params.size() == 1);
  REQUIRE(seeds[0].getHits().size() == 3);

  const auto param = params[0];
  CHECK(std::isfinite(param.getPhi()));
  CHECK(std::isfinite(param.getTheta()));
  CHECK(std::isfinite(param.getQOverP()));
  CHECK(param.getTheta() == Catch::Approx(1.0666F).epsilon(0.1F));
  CHECK(param.getQOverP() != Catch::Approx(0.0F));
  CHECK(seeds[0].getParams().getQOverP() == Catch::Approx(param.getQOverP()));
}

void checkAcceptedSpacePointEmpty(TrackSeedingConfig::SeedingMethod seedingMethod,
                                  std::string_view algoName) {
  TrackSeeding algo(algoName);
  TrackSeedingConfig cfg;
  cfg.seedingMethod = seedingMethod;
  cfg.rMin          = 0.0F;
  cfg.rMax          = 1.0F;
  cfg.zMin          = -1.0F;
  cfg.zMax          = 1.0F;
  algo.applyConfig(cfg);
  algo.init();

  edm4eic::TrackerHitCollection hits;
  hits.create(1, edm4hep::Vector3f(50.0F, 0.0F, 10.0F), edm4eic::CovDiag3f(), 0.0F, 0.0F, 1.0F,
              0.0F);
  hits.create(2, edm4hep::Vector3f(60.0F, 0.0F, 20.0F), edm4eic::CovDiag3f(), 0.0F, 0.0F, 1.0F,
              0.0F);

  edm4eic::TrackSeedCollection seeds;
  edm4eic::TrackParametersCollection params;
  algo.process({&hits}, {&seeds, &params});

  REQUIRE(seeds.empty());
  REQUIRE(params.empty());
}

#if TRACKSEEDING_HAS_SEEDING2 || TRACKSEEDING_HAS_SEEDING
/// Number of Seeding2 seeds for a triplet whose r-z segment angles differ by ~20 mrad.
std::size_t countSeedsForMisalignedTriplet(float deltaThetaMax, std::string_view algoName) {
  TrackSeeding algo(algoName);
  TrackSeedingConfig cfg = threeHitConfig(TrackSeedingConfig::SeedingMethod::Seeding2);
  cfg.deltaThetaMax      = deltaThetaMax;
  algo.applyConfig(cfg);
  algo.init();

  // theta(bottom->middle) = atan2(53.85 - 33.54, 20 - 10)      = 1.1133 rad
  // theta(middle->top)    = atan2(71.87 - 53.85, 28.443 - 20)  = 1.1326 rad
  // -> |dTheta| = 19 mrad, well inside the (very loose) scattering window of the
  // toy configuration, so only deltaThetaMax decides whether the seed survives.
  const edm4eic::TrackerHitCollection hits = threeHits(28.443F);

  edm4eic::TrackSeedCollection seeds;
  edm4eic::TrackParametersCollection params;
  algo.process({&hits}, {&seeds, &params});
  REQUIRE(params.size() == seeds.size());
  return seeds.size();
}
#endif

} // namespace

#if TRACKSEEDING_HAS_SEEDING2 || TRACKSEEDING_HAS_SEEDING
TEST_CASE("TrackSeeding (Seeding2): three hits produce one seed with stable parameters",
          "[TrackSeeding]") {
  checkThreeHitsProduceOneSeed(TrackSeedingConfig::SeedingMethod::Seeding2,
                               "test_track_seeding_seeding2");
}

TEST_CASE("TrackSeeding (Seeding2): accepted-spacepoint-empty path exits cleanly",
          "[TrackSeeding]") {
  checkAcceptedSpacePointEmpty(TrackSeedingConfig::SeedingMethod::Seeding2,
                               "test_track_seeding_seeding2_empty");
}

TEST_CASE("TrackSeeding (Seeding2): deltaThetaMax rejects r-z misaligned triplets",
          "[TrackSeeding]") {
  // default 5 mrad (Orthogonal-equivalent) rejects a 20 mrad kink
  CHECK(countSeedsForMisalignedTriplet(0.005F, "test_track_seeding_seeding2_dtheta_default") == 0);
  // a looser cut accepts it
  CHECK(countSeedsForMisalignedTriplet(0.05F, "test_track_seeding_seeding2_dtheta_loose") == 1);
  // deltaThetaMax <= 0 disables the cut
  CHECK(countSeedsForMisalignedTriplet(0.0F, "test_track_seeding_seeding2_dtheta_off") == 1);
}
#endif

#if TRACKSEEDING_HAS_ORTHOGONAL
TEST_CASE("TrackSeeding (Orthogonal): three hits produce one seed with stable parameters",
          "[TrackSeeding]") {
  checkThreeHitsProduceOneSeed(TrackSeedingConfig::SeedingMethod::Orthogonal,
                               "test_track_seeding_orthogonal");
}

TEST_CASE("TrackSeeding (Orthogonal): accepted-spacepoint-empty path exits cleanly",
          "[TrackSeeding]") {
  checkAcceptedSpacePointEmpty(TrackSeedingConfig::SeedingMethod::Orthogonal,
                               "test_track_seeding_orthogonal_empty");
}
#endif
