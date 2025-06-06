// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Christopher Dilks

#include <algorithms/logger.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cmath>
#include <edm4eic/TrackPoint.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4hep/Vector3f.h>
#include <gsl/pointers>
#include <memory>
#include <vector>

#include "algorithms/pid/MergeTracks.h"

TEST_CASE("the PID MergeTracks algorithm runs", "[MergeTracks]") {
  eicrecon::MergeTracks algo("test");

  // initialize algorithm
  //----------------------------------------------------------
  algo.level(algorithms::LogLevel::kDebug);
  algo.init();

  // helper functions and objects
  //----------------------------------------------------------
  const float EPSILON = 1e-5;

  struct Point {
    decltype(edm4eic::TrackPoint::position) position;
    decltype(edm4eic::TrackPoint::momentum) momentum;
    decltype(edm4eic::TrackPoint::time) time;
  };

  auto make_track = [](auto& coll, std::vector<Point> points) {
    auto trk = coll->create();
    for (auto& point : points) {
      edm4eic::TrackPoint trk_point;
      trk_point.position = point.position;
      trk_point.momentum = point.momentum;
      trk_point.time     = point.time;
      trk.addToPoints(trk_point);
    }
  };

  auto track_length = [](auto trk) {
    auto a = trk.getPoints(0);
    auto b = trk.getPoints(trk.points_size() - 1);
    return std::hypot(a.position.x - b.position.x, a.position.y - b.position.y,
                      a.position.z - b.position.z);
  };

  // inputs
  //----------------------------------------------------------
  /*
     |        | collection0 | collection1 | collection2 |
     | ------ | ----------- | ----------- | ----------- |
     | track0 | horizontal  | horizontal  | horizontal  |
     | track1 | horizontal  | empty       | one point   |
     | track2 | vertical    | horizontal  | vertical    |
     | track3 | horizontal  | horizontal  | horizontal  | // time disordered
  */

  // collections
  auto collection0 = std::make_unique<edm4eic::TrackSegmentCollection>();
  auto collection1 = std::make_unique<edm4eic::TrackSegmentCollection>();
  auto collection2 = std::make_unique<edm4eic::TrackSegmentCollection>();

  // track0
  make_track(collection0, {                           // horizontal
                           {{0, 0, 0}, {1, 0, 0}, 1}, // { position, momentum, time }
                           {{1, 0, 0}, {1, 0, 0}, 2}});
  make_track(collection1, {// horizontal
                           {{2, 0, 0}, {1, 0, 0}, 3},
                           {{3, 0, 0}, {1, 0, 0}, 4}});
  make_track(collection2, {// horizontal
                           {{4, 0, 0}, {1, 0, 0}, 5},
                           {{5, 0, 0}, {1, 0, 0}, 6}});

  // track1
  make_track(collection0, {// horizontal
                           {{0, 0, 0}, {1, 0, 0}, 1},
                           {{1, 0, 0}, {1, 0, 0}, 2}});
  make_track(collection1, {// empty
                           {}});
  make_track(collection2, {// one point
                           {{2, 0, 0}, {1, 0, 0}, 3}});

  // track2
  make_track(collection0, {// vertical
                           {{0, 0, 0}, {0, 1, 0}, 1},
                           {{0, 1, 0}, {0, 1, 0}, 2}});
  make_track(collection1, {// horizontal
                           {{0, 1, 0}, {1, 0, 0}, 3},
                           {{1, 1, 0}, {1, 0, 0}, 4}});
  make_track(collection2, {// vertical
                           {{1, 1, 0}, {0, 1, 0}, 5},
                           {{1, 2, 0}, {0, 1, 0}, 6}});

  // track3
  make_track(collection0, {// horizontal
                           {{1, 0, 0}, {1, 0, 0}, 2},
                           {{0, 0, 0}, {1, 0, 0}, 1}});
  make_track(collection1, {// horizontal
                           {{3, 0, 0}, {1, 0, 0}, 4},
                           {{4, 0, 0}, {1, 0, 0}, 5}});
  make_track(collection2, {// horizontal
                           {{5, 0, 0}, {1, 0, 0}, 6},
                           {{2, 0, 0}, {1, 0, 0}, 3}});

  // tests
  //----------------------------------------------------------

  SECTION("merge tracks from 1 collection") {
    // run algorithm
    std::vector<gsl::not_null<const edm4eic::TrackSegmentCollection*>> colls = {collection0.get()};
    // create output collection
    auto trks = std::make_unique<edm4eic::TrackSegmentCollection>();
    algo.process({colls}, {trks.get()});
    // input collection(s) size == output collection size
    REQUIRE(trks->size() == colls.front()->size());
    // track length: from endpoints
    REQUIRE_THAT(track_length(trks->at(0)), Catch::Matchers::WithinAbs(1, EPSILON));
    REQUIRE_THAT(track_length(trks->at(1)), Catch::Matchers::WithinAbs(1, EPSILON));
    REQUIRE_THAT(track_length(trks->at(2)), Catch::Matchers::WithinAbs(1, EPSILON));
    REQUIRE_THAT(track_length(trks->at(3)), Catch::Matchers::WithinAbs(1, EPSILON));
    // track length: from algorithm // FIXME when implemented in `MergeTracks`
    for (const auto& trk : *trks) {
      REQUIRE_THAT(trk.getLength(), Catch::Matchers::WithinAbs(0, EPSILON));
    }
  }

  SECTION("merge tracks from 2 collections") {
    // run algorithm
    std::vector<gsl::not_null<const edm4eic::TrackSegmentCollection*>> colls = {collection0.get(),
                                                                                collection1.get()};
    auto trks = std::make_unique<edm4eic::TrackSegmentCollection>();
    algo.process({colls}, {trks.get()});
    // input collection(s) size == output collection size
    REQUIRE(trks->size() == colls.front()->size());
    // track length: from endpoints
    REQUIRE_THAT(track_length(trks->at(0)), Catch::Matchers::WithinAbs(3, EPSILON));
    REQUIRE_THAT(track_length(trks->at(1)), Catch::Matchers::WithinAbs(1, EPSILON));
    REQUIRE_THAT(track_length(trks->at(2)), Catch::Matchers::WithinAbs(std::hypot(1, 1), EPSILON));
    REQUIRE_THAT(track_length(trks->at(3)), Catch::Matchers::WithinAbs(4, EPSILON));
    // track length: from algorithm // FIXME when implemented in `MergeTracks`
    for (const auto& trk : *trks) {
      REQUIRE_THAT(trk.getLength(), Catch::Matchers::WithinAbs(0, EPSILON));
    }
  }

  SECTION("merge tracks from 3 collections") {
    // run algorithm
    std::vector<gsl::not_null<const edm4eic::TrackSegmentCollection*>> colls = {
        collection0.get(), collection1.get(), collection2.get()};
    auto trks = std::make_unique<edm4eic::TrackSegmentCollection>();
    algo.process({colls}, {trks.get()});
    // input collection(s) size == output collection size
    REQUIRE(trks->size() == colls.front()->size());
    // track length: from endpoints
    REQUIRE_THAT(track_length(trks->at(0)), Catch::Matchers::WithinAbs(5, EPSILON));
    REQUIRE_THAT(track_length(trks->at(1)), Catch::Matchers::WithinAbs(2, EPSILON));
    REQUIRE_THAT(track_length(trks->at(2)), Catch::Matchers::WithinAbs(std::hypot(1, 2), EPSILON));
    REQUIRE_THAT(track_length(trks->at(3)), Catch::Matchers::WithinAbs(5, EPSILON));
    // track length: from algorithm // FIXME when implemented in `MergeTracks`
    for (const auto& trk : *trks) {
      REQUIRE_THAT(trk.getLength(), Catch::Matchers::WithinAbs(0, EPSILON));
    }
  }
}
