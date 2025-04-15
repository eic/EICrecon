// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 - 2025, Dmitry Kalinkin, Simon Gardner

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <algorithms/geo.h>
#include <algorithms/logger.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <edm4eic/CovDiag3f.h>
#include <edm4eic/Measurement2DCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/Vector2f.h>
#include <edm4hep/Vector3f.h>
#include <podio/RelationRange.h>
#include <gsl/pointers>
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
    edm4eic::TrackerHitCollection hits_coll;
    hits_coll.create(id_desc.encode({{"system", 255}, {"x", 0}, {"y", 0}}), // std::uint64_t cellID,
                     edm4hep::Vector3f(0.0, 0.0, 0.0),                      // Vector3f position,
                     edm4eic::CovDiag3f(),                                  // Cov3f cov,
                     0.0,                                                   // float time
                     0.0,                                                   // float timeError,
                     5.0,                                                   // float edep,
                     0.0                                                    // float edepError
    );

    edm4eic::Measurement2DCollection clusterPositions;
    algo.ClusterHits(hits_coll, clusterPositions);

    REQUIRE(clusterPositions.size() == 1);
    REQUIRE(clusterPositions[0].getHits().size() == 1);
    REQUIRE(clusterPositions[0].getLoc()[0] == 0.0);
    REQUIRE(clusterPositions[0].getLoc()[1] == 0.0);
    REQUIRE(clusterPositions[0].getTime() == 0.0);
  }

  SECTION("on two separated pixels") {
    edm4eic::TrackerHitCollection hits_coll;
    hits_coll.create(
        id_desc.encode({{"system", 255}, {"x", 0}, {"y", 10}}), // std::uint64_t cellID,
        edm4hep::Vector3f(0.0, 0.0, 0.0),                       // Vector3f position,
        edm4eic::CovDiag3f(),                                   // Cov3f cov,
        5.0,                                                    // float time
        0.0,                                                    // float timeError,
        5.0,                                                    // float edep,
        0.0                                                     // float edepError
    );
    hits_coll.create(
        id_desc.encode({{"system", 255}, {"x", 10}, {"y", 0}}), // std::uint64_t cellID,
        edm4hep::Vector3f(0.0, 0.0, 0.0),                       // Vector3f position,
        edm4eic::CovDiag3f(),                                   // Cov3f cov,
        5.0,                                                    // float time
        0.0,                                                    // float timeError,
        5.0,                                                    // float edep,
        0.0                                                     // float edepError
    );

    edm4eic::Measurement2DCollection clusterPositions;
    algo.ClusterHits(hits_coll, clusterPositions);

    REQUIRE(clusterPositions.size() == 2);
    REQUIRE(clusterPositions[0].getHits().size() == 1);
    REQUIRE(clusterPositions[1].getHits().size() == 1);
  }

  SECTION("on two adjacent pixels") {
    edm4eic::TrackerHitCollection hits_coll;
    hits_coll.create(id_desc.encode({{"system", 255}, {"x", 0}, {"y", 0}}), // std::uint64_t cellID,
                     edm4hep::Vector3f(0.0, 0.0, 0.0),                      // Vector3f position,
                     edm4eic::CovDiag3f(),                                  // Cov3f cov,
                     5.0,                                                   // float time
                     0.0,                                                   // float timeError,
                     5.0,                                                   // float edep,
                     0.0                                                    // float edepError
    );
    hits_coll.create(id_desc.encode({{"system", 255}, {"x", 1}, {"y", 0}}), // std::uint64_t cellID,
                     edm4hep::Vector3f(0.0, 0.0, 0.0),                      // Vector3f position,
                     edm4eic::CovDiag3f(),                                  // Cov3f cov,
                     5.0,                                                   // float time
                     0.0,                                                   // float timeError,
                     5.0,                                                   // float edep,
                     0.0                                                    // float edepError
    );

    edm4eic::Measurement2DCollection clusterPositions;
    algo.ClusterHits(hits_coll, clusterPositions);

    REQUIRE(clusterPositions.size() == 1);
    REQUIRE(clusterPositions[0].getHits().size() == 2);
    REQUIRE(clusterPositions[0].getLoc()[0] == 0.5);
  }

  SECTION("on two adjacent pixels outwith the time separation") {
    edm4eic::TrackerHitCollection hits_coll;
    hits_coll.create(id_desc.encode({{"system", 255}, {"x", 0}, {"y", 0}}), // std::uint64_t cellID,
                     edm4hep::Vector3f(0.0, 0.0, 0.0),                      // Vector3f position,
                     edm4eic::CovDiag3f(),                                  // Cov3f cov,
                     0.0,                                                   // float time
                     0.0,                                                   // float timeError,
                     5.0,                                                   // float edep,
                     0.0                                                    // float edepError
    );
    hits_coll.create(id_desc.encode({{"system", 255}, {"x", 1}, {"y", 0}}), // std::uint64_t cellID,
                     edm4hep::Vector3f(0.0, 0.0, 0.0),                      // Vector3f position,
                     edm4eic::CovDiag3f(),                                  // Cov3f cov,
                     1.1 * cfg.hit_time_limit,                              // float time
                     0.0,                                                   // float timeError,
                     5.0,                                                   // float edep,
                     0.0                                                    // float edepError
    );

    edm4eic::Measurement2DCollection clusterPositions;
    algo.ClusterHits(hits_coll, clusterPositions);

    REQUIRE(clusterPositions.size() == 2);
    REQUIRE(clusterPositions[0].getHits().size() == 1);
    REQUIRE(clusterPositions[1].getHits().size() == 1);
  }

  SECTION("run on three adjacent pixels") {

    // Check I and L shape clusters
    auto pixel3       = GENERATE(std::vector<int>{2, 0}, std::vector<int>{1, 1});
    auto pixelCharges = GENERATE(std::vector<int>{5, 10, 5}, std::vector<int>{10, 5, 5});
    float pixel2Time  = GENERATE_COPY(0, 1.1 * cfg.hit_time_limit);

    edm4eic::TrackerHitCollection hits_coll;
    hits_coll.create(id_desc.encode({{"system", 255}, {"x", 0}, {"y", 0}}), // std::uint64_t cellID,
                     edm4hep::Vector3f(0.0, 0.0, 0.0),                      // Vector3f position,
                     edm4eic::CovDiag3f(),                                  // Cov3f cov,
                     0.0,                                                   // float time
                     0.0,                                                   // float timeError,
                     pixelCharges[0],                                       // float edep,
                     0.0                                                    // float edepError
    );
    hits_coll.create(id_desc.encode({{"system", 255}, {"x", 1}, {"y", 0}}), // std::uint64_t cellID,
                     edm4hep::Vector3f(0.0, 0.0, 0.0),                      // Vector3f position,
                     edm4eic::CovDiag3f(),                                  // Cov3f cov,
                     pixel2Time,                                            // float time
                     0.0,                                                   // float timeError,
                     pixelCharges[1],                                       // float edep,
                     0.0                                                    // float edepError
    );
    hits_coll.create(
        id_desc.encode(
            {{"system", 255}, {"x", pixel3[0]}, {"y", pixel3[1]}}), // std::uint64_t cellID,
        edm4hep::Vector3f(0.0, 0.0, 0.0),                           // Vector3f position,
        edm4eic::CovDiag3f(),                                       // Cov3f cov,
        0.0,                                                        // float time
        0.0,                                                        // float timeError,
        pixelCharges[2],                                            // float edep,
        0.0                                                         // float edepError
    );

    edm4eic::Measurement2DCollection clusterPositions;
    algo.ClusterHits(hits_coll, clusterPositions);

    if (pixel2Time < cfg.hit_time_limit) {
      REQUIRE(clusterPositions.size() == 1);
      REQUIRE(clusterPositions[0].getHits().size() == 3);
    } else if (pixel3[0] == 2) {
      REQUIRE(clusterPositions.size() == 3);
      REQUIRE(clusterPositions[0].getHits().size() == 1);
      REQUIRE(clusterPositions[1].getHits().size() == 1);
      REQUIRE(clusterPositions[2].getHits().size() == 1);
    } else {
      REQUIRE(clusterPositions.size() == 2);
    }
  }
}
