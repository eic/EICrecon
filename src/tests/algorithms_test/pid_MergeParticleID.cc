// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Christopher Dilks

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <edm4eic/CherenkovParticleIDCollection.h>
#include <edm4eic/CherenkovParticleIDHypothesis.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4hep/Vector2f.h>
#include <podio/RelationRange.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <cmath>
#include <gsl/pointers>
#include <memory>
#include <stdexcept>
#include <vector>

#include "algorithms/pid/MergeParticleID.h"
#include "algorithms/pid/MergeParticleIDConfig.h"

TEST_CASE("the PID MergeParticleID algorithm runs", "[MergeParticleID]") {

  const float EPSILON = 1e-5;

  std::shared_ptr<spdlog::logger> logger = spdlog::default_logger()->clone("MergeParticleID");
  logger->set_level(spdlog::level::debug);

  // charged particle input data
  //----------------------------------------------------------
  /* just create two unique tracks; they don't have to be filled with data,
   * since `MergeParticleID` matches tracks by ID
   */
  auto tracks = std::make_unique<edm4eic::TrackSegmentCollection>();
  for (int i = 0; i < 2; i++) {
    tracks->create();
  }

  // Cherenkov PID inputs
  //----------------------------------------------------------
  /* 2 collections, each with 2 elements corresponding to 2 charged particles;
   * 2 or 3 possible PID hypotheses for each
   */

  edm4eic::MutableCherenkovParticleID pid;
  edm4eic::CherenkovParticleIDHypothesis pid_hyp;

  // collection 1 ------
  auto coll_cherenkov_1 = std::make_unique<edm4eic::CherenkovParticleIDCollection>();

  pid = coll_cherenkov_1->create();
  pid.setChargedParticle(tracks->at(0)); // track 0 first
  pid.setNpe(10);
  pid.setRefractiveIndex(1.05);
  pid.setPhotonEnergy(3e-9);
  pid_hyp.PDG    = 211; // pion hypothesis first
  pid_hyp.npe    = 10;
  pid_hyp.weight = 100;
  pid.addToHypotheses(pid_hyp);
  pid_hyp.PDG    = 321;
  pid_hyp.npe    = 8;
  pid_hyp.weight = 80;
  pid.addToHypotheses(pid_hyp);
  for (int i = 0; i <= pid.getNpe(); i++) {
    pid.addToThetaPhiPhotons(edm4hep::Vector2f{M_PI / 4, 0});
  }

  pid = coll_cherenkov_1->create();
  pid.setChargedParticle(tracks->at(1));
  pid.setNpe(11);
  pid.setRefractiveIndex(1.06);
  pid.setPhotonEnergy(4e-9);
  pid_hyp.PDG    = 321; // kaon hypothesis first
  pid_hyp.npe    = 10;
  pid_hyp.weight = 100;
  pid.addToHypotheses(pid_hyp);
  pid_hyp.PDG    = 211;
  pid_hyp.npe    = 10;
  pid_hyp.weight = 80;
  pid.addToHypotheses(pid_hyp);
  for (int i = 0; i <= pid.getNpe(); i++) {
    pid.addToThetaPhiPhotons(edm4hep::Vector2f{-M_PI / 4, 0});
  }

  // collection 2 ------
  auto coll_cherenkov_2 = std::make_unique<edm4eic::CherenkovParticleIDCollection>();

  pid = coll_cherenkov_2->create();
  pid.setChargedParticle(tracks->at(1)); // track 1 first
  pid.setNpe(4);
  pid.setRefractiveIndex(1.5);
  pid.setPhotonEnergy(3e-9);
  pid_hyp.PDG    = 211;
  pid_hyp.npe    = 3;
  pid_hyp.weight = 200;
  pid.addToHypotheses(pid_hyp);
  pid_hyp.PDG    = 321;
  pid_hyp.npe    = 2;
  pid_hyp.weight = 90;
  pid.addToHypotheses(pid_hyp);
  for (int i = 0; i <= pid.getNpe(); i++) {
    pid.addToThetaPhiPhotons(edm4hep::Vector2f{M_PI / 4, 0});
  }

  pid = coll_cherenkov_2->create();
  pid.setChargedParticle(tracks->at(0));
  pid.setNpe(6);
  pid.setRefractiveIndex(1.3);
  pid.setPhotonEnergy(4e-9);
  pid_hyp.PDG    = 321;
  pid_hyp.npe    = 4;
  pid_hyp.weight = 70;
  pid.addToHypotheses(pid_hyp);
  pid_hyp.PDG    = 211;
  pid_hyp.npe    = 1;
  pid_hyp.weight = 80;
  pid.addToHypotheses(pid_hyp);
  pid_hyp.PDG    = 11; // an additional hypothesis
  pid_hyp.npe    = 1;
  pid_hyp.weight = 60;
  pid.addToHypotheses(pid_hyp);
  for (int i = 0; i <= pid.getNpe(); i++) {
    pid.addToThetaPhiPhotons(edm4hep::Vector2f{-M_PI / 4, 0});
  }

  // Cherenkov PID tests
  //----------------------------------------------------------

  std::vector<gsl::not_null<const edm4eic::CherenkovParticleIDCollection*>> coll_cherenkov_list = {
      coll_cherenkov_1.get(), coll_cherenkov_2.get()};

  auto find_cherenkov_pid_for_track = [](const auto& coll, auto trk) {
    for (auto obj : *coll) {
      if (obj.getChargedParticle().id() == trk.id()) {
        return obj;
      }
    }
    FAIL("ERROR: cannot find CherenkovParticleID given track");
    if (coll->size() == 0) {
      throw std::runtime_error(
          "empty collection used in pid_MergeParticleID::find_cherenkov_pid_for_track");
    }
    return coll->at(0);
  };

  // additive weights
  SECTION("merge CherenkovParticleID: add hypothesis weights") {

    eicrecon::MergeParticleID algo("test");
    eicrecon::MergeParticleIDConfig cfg;
    cfg.mergeMode = eicrecon::MergeParticleIDConfig::kAddWeights;
    algo.applyConfig(cfg);
    algo.init();

    auto result = std::make_unique<edm4eic::CherenkovParticleIDCollection>();
    algo.process({coll_cherenkov_list}, {result.get()});
    auto pid_0 = find_cherenkov_pid_for_track(result, tracks->at(0));
    auto pid_1 = find_cherenkov_pid_for_track(result, tracks->at(1));

    REQUIRE_THAT(pid_0.getNpe(), Catch::Matchers::WithinAbs(10 + 6, EPSILON));
    REQUIRE_THAT(pid_1.getNpe(), Catch::Matchers::WithinAbs(11 + 4, EPSILON));

    REQUIRE_THAT(pid_0.getRefractiveIndex(),
                 Catch::Matchers::WithinAbs((10 * 1.05 + 6 * 1.3) / (10 + 6), EPSILON));
    REQUIRE_THAT(pid_1.getRefractiveIndex(),
                 Catch::Matchers::WithinAbs((11 * 1.06 + 4 * 1.5) / (11 + 4), EPSILON));

    REQUIRE_THAT(pid_0.getPhotonEnergy(),
                 Catch::Matchers::WithinAbs((10 * 3e-9 + 6 * 4e-9) / (10 + 6), EPSILON));
    REQUIRE_THAT(pid_1.getPhotonEnergy(),
                 Catch::Matchers::WithinAbs((11 * 4e-9 + 4 * 3e-9) / (11 + 4), EPSILON));

    REQUIRE(pid_0.hypotheses_size() == 3);
    for (auto hyp : pid_0.getHypotheses()) {
      switch (hyp.PDG) {
      case 211:
        REQUIRE_THAT(hyp.npe, Catch::Matchers::WithinAbs(10 + 1, EPSILON));
        REQUIRE_THAT(hyp.weight, Catch::Matchers::WithinAbs(100 + 80, EPSILON));
        break;
      case 321:
        REQUIRE_THAT(hyp.npe, Catch::Matchers::WithinAbs(8 + 4, EPSILON));
        REQUIRE_THAT(hyp.weight, Catch::Matchers::WithinAbs(80 + 70, EPSILON));
        break;
      case 11:
        REQUIRE_THAT(hyp.npe, Catch::Matchers::WithinAbs(1, EPSILON));
        REQUIRE_THAT(hyp.weight, Catch::Matchers::WithinAbs(60, EPSILON));
        break;
      default:
        FAIL("untested PDG hypothesis");
      }
    }

    REQUIRE(pid_1.hypotheses_size() == 2);
    for (auto hyp : pid_1.getHypotheses()) {
      switch (hyp.PDG) {
      case 211:
        REQUIRE_THAT(hyp.npe, Catch::Matchers::WithinAbs(10 + 3, EPSILON));
        REQUIRE_THAT(hyp.weight, Catch::Matchers::WithinAbs(80 + 200, EPSILON));
        break;
      case 321:
        REQUIRE_THAT(hyp.npe, Catch::Matchers::WithinAbs(10 + 2, EPSILON));
        REQUIRE_THAT(hyp.weight, Catch::Matchers::WithinAbs(100 + 90, EPSILON));
        break;
      default:
        FAIL("untested PDG hypothesis");
      }
    }
  }

  // multiplicative weights
  /* NOTE: the only difference from additive weights is the resulting weight values, but it
   * is still wise to re-test everything, so that any changes in the algorithm require corresponding
   * changes in these tests
   */
  SECTION("merge CherenkovParticleID: multiply hypothesis weights") {

    eicrecon::MergeParticleID algo("test");
    eicrecon::MergeParticleIDConfig cfg;
    cfg.mergeMode = eicrecon::MergeParticleIDConfig::kMultiplyWeights;
    algo.applyConfig(cfg);
    algo.init();

    auto result = std::make_unique<edm4eic::CherenkovParticleIDCollection>();
    algo.process({coll_cherenkov_list}, {result.get()});
    auto pid_0 = find_cherenkov_pid_for_track(result, tracks->at(0));
    auto pid_1 = find_cherenkov_pid_for_track(result, tracks->at(1));

    REQUIRE_THAT(pid_0.getNpe(), Catch::Matchers::WithinAbs(10 + 6, EPSILON));
    REQUIRE_THAT(pid_1.getNpe(), Catch::Matchers::WithinAbs(11 + 4, EPSILON));

    REQUIRE_THAT(pid_0.getRefractiveIndex(),
                 Catch::Matchers::WithinAbs((10 * 1.05 + 6 * 1.3) / (10 + 6), EPSILON));
    REQUIRE_THAT(pid_1.getRefractiveIndex(),
                 Catch::Matchers::WithinAbs((11 * 1.06 + 4 * 1.5) / (11 + 4), EPSILON));

    REQUIRE_THAT(pid_0.getPhotonEnergy(),
                 Catch::Matchers::WithinAbs((10 * 3e-9 + 6 * 4e-9) / (10 + 6), EPSILON));
    REQUIRE_THAT(pid_1.getPhotonEnergy(),
                 Catch::Matchers::WithinAbs((11 * 4e-9 + 4 * 3e-9) / (11 + 4), EPSILON));

    for (auto hyp : pid_0.getHypotheses()) {
      switch (hyp.PDG) {
      case 211:
        REQUIRE_THAT(hyp.npe, Catch::Matchers::WithinAbs(10 + 1, EPSILON));
        REQUIRE_THAT(hyp.weight, Catch::Matchers::WithinAbs(100 * 80, EPSILON));
        break;
      case 321:
        REQUIRE_THAT(hyp.npe, Catch::Matchers::WithinAbs(8 + 4, EPSILON));
        REQUIRE_THAT(hyp.weight, Catch::Matchers::WithinAbs(80 * 70, EPSILON));
        break;
      case 11:
        REQUIRE_THAT(hyp.npe, Catch::Matchers::WithinAbs(1, EPSILON));
        REQUIRE_THAT(hyp.weight, Catch::Matchers::WithinAbs(60, EPSILON));
        break;
      default:
        FAIL("untested PDG hypothesis");
      }
    }

    for (auto hyp : pid_1.getHypotheses()) {
      switch (hyp.PDG) {
      case 211:
        REQUIRE_THAT(hyp.npe, Catch::Matchers::WithinAbs(10 + 3, EPSILON));
        REQUIRE_THAT(hyp.weight, Catch::Matchers::WithinAbs(80 * 200, EPSILON));
        break;
      case 321:
        REQUIRE_THAT(hyp.npe, Catch::Matchers::WithinAbs(10 + 2, EPSILON));
        REQUIRE_THAT(hyp.weight, Catch::Matchers::WithinAbs(100 * 90, EPSILON));
        break;
      default:
        FAIL("untested PDG hypothesis");
      }
    }
  }
}
