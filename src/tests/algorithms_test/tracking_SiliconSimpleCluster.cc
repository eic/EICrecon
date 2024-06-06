// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Dmitry Kalinkin, Simon Gardner

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <algorithms/geo.h>
#include <algorithms/logger.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/unit_system.h>
#include <gsl/pointers>
#include <podio/ObjectID.h>
#include <utility>
#include <vector>

#include "algorithms/fardetectors/FarDetectorTrackerCluster.h"
#include "algorithms/fardetectors/FarDetectorTrackerClusterConfig.h"

TEST_CASE("the clustering algorithm runs", "[FarDetectorTrackerCluster]") {
  eicrecon::FarDetectorTrackerCluster algo("FarDetectorTrackerCluster");

  eicrecon::FarDetectorTrackerClusterConfig cfg;
  cfg.hit_time_limit = 10.0 * edm4eic::unit::ns;
  cfg.readout        = "MockTrackerHits";
  cfg.x_field        = "x";
  cfg.y_field        = "y";

  auto detector = algorithms::GeoSvc::instance().detector();
  auto id_desc  = detector->readout(cfg.readout).idSpec();

  algo.applyConfig(cfg);
  algo.level(algorithms::LogLevel::kTrace);
  algo.init();

  SECTION("on a single pixel") {
    edm4eic::RawTrackerHitCollection hits_coll;
    hits_coll.create(id_desc.encode({{"system", 255}, {"x", 0}, {"y", 0}}), // std::uint64_t cellID,
                     5.0,                                                   // int32 charge,
                     0.0                                                    // int32 timeStamp
    );

    std::vector<FDTrackerCluster> clusterPositions = algo.ClusterHits(hits_coll);

    REQUIRE(clusterPositions.size() == 1);
    REQUIRE(clusterPositions[0].rawHits.size() == 1);
    REQUIRE(clusterPositions[0].x == 0.0);
    REQUIRE(clusterPositions[0].y == 0.0);
    REQUIRE(clusterPositions[0].energy == 5.0);
    REQUIRE(clusterPositions[0].time == 0.0);
  }

  SECTION("on two separated pixels") {
    edm4eic::RawTrackerHitCollection hits_coll;
    hits_coll.create(
        id_desc.encode({{"system", 255}, {"x", 0}, {"y", 10}}), // std::uint64_t cellID,
        5.0,                                                    // int32 charge,
        5.0                                                     // int32 timeStamp
    );
    hits_coll.create(
        id_desc.encode({{"system", 255}, {"x", 10}, {"y", 0}}), // std::uint64_t cellID,
        5.0,                                                    // int32 charge,
        5.0                                                     // int32 timeStamp
    );

    std::vector<FDTrackerCluster> clusterPositions = algo.ClusterHits(hits_coll);

    REQUIRE(clusterPositions.size() == 2);
    REQUIRE(clusterPositions[0].rawHits.size() == 1);
    REQUIRE(clusterPositions[1].rawHits.size() == 1);
  }

  SECTION("on two adjacent pixels") {
    edm4eic::RawTrackerHitCollection hits_coll;
    hits_coll.create(id_desc.encode({{"system", 255}, {"x", 0}, {"y", 0}}), // std::uint64_t cellID,
                     5.0,                                                   // int32 charge,
                     5.0                                                    // int32 timeStamp
    );
    hits_coll.create(id_desc.encode({{"system", 255}, {"x", 1}, {"y", 0}}), // std::uint64_t cellID,
                     5.0,                                                   // int32 charge,
                     5.0                                                    // int32 timeStamp
    );

    std::vector<FDTrackerCluster> clusterPositions = algo.ClusterHits(hits_coll);

    REQUIRE(clusterPositions.size() == 1);
    REQUIRE(clusterPositions[0].rawHits.size() == 2);
    REQUIRE(clusterPositions[0].x == 0.5);
  }

  SECTION("on two adjacent pixels outwith the time separation") {
    edm4eic::RawTrackerHitCollection hits_coll;
    hits_coll.create(id_desc.encode({{"system", 255}, {"x", 0}, {"y", 0}}), // std::uint64_t cellID,
                     5.0,                                                   // int32 charge,
                     0.0                                                    // int32 timeStamp
    );
    hits_coll.create(id_desc.encode({{"system", 255}, {"x", 1}, {"y", 0}}), // std::uint64_t cellID,
                     5.0,                                                   // int32 charge,
                     1.1 * cfg.hit_time_limit                               // int32 timeStamp
    );

    std::vector<FDTrackerCluster> clusterPositions = algo.ClusterHits(hits_coll);

    REQUIRE(clusterPositions.size() == 2);
    REQUIRE(clusterPositions[0].rawHits.size() == 1);
    REQUIRE(clusterPositions[1].rawHits.size() == 1);
  }

  SECTION("run on three adjacent pixels") {

    // Check I and L shape clusters
    auto pixel3       = GENERATE(std::vector<int>{2, 0}, std::vector<int>{1, 1});
    auto pixelCharges = GENERATE(std::vector<int>{5, 10, 5}, std::vector<int>{10, 5, 5});
    float pixel2Time  = GENERATE_COPY(0, 1.1 * cfg.hit_time_limit);

    edm4eic::RawTrackerHitCollection hits_coll;
    hits_coll.create(id_desc.encode({{"system", 255}, {"x", 0}, {"y", 0}}), // std::uint64_t cellID,
                     pixelCharges[0],                                       // int32 charge,
                     0.0                                                    // int32 timeStamp
    );
    hits_coll.create(id_desc.encode({{"system", 255}, {"x", 1}, {"y", 0}}), // std::uint64_t cellID,
                     pixelCharges[1],                                       // int32 charge,
                     pixel2Time                                             // int32 timeStamp
    );
    hits_coll.create(
        id_desc.encode(
            {{"system", 255}, {"x", pixel3[0]}, {"y", pixel3[1]}}), // std::uint64_t cellID,
        pixelCharges[2],                                            // int32 charge,
        0.0                                                         // int32 timeStamp
    );

    std::vector<FDTrackerCluster> clusterPositions = algo.ClusterHits(hits_coll);

    if (pixel2Time < cfg.hit_time_limit) {
      REQUIRE(clusterPositions.size() == 1);
      REQUIRE(clusterPositions[0].rawHits.size() == 3);
    } else if (pixel3[0] == 2) {
      REQUIRE(clusterPositions.size() == 3);
      REQUIRE(clusterPositions[0].rawHits.size() == 1);
      REQUIRE(clusterPositions[1].rawHits.size() == 1);
      REQUIRE(clusterPositions[2].rawHits.size() == 1);
    } else {
      REQUIRE(clusterPositions.size() == 2);
    }
  }
}
