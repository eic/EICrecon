// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Dmitry Kalinkin

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <Evaluator/DD4hepUnits.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/ProtoClusterCollection.h>
#include <edm4hep/Vector3f.h>
#include <podio/RelationRange.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/calorimetry/CalorimeterIslandCluster.h"
#include "algorithms/calorimetry/CalorimeterIslandClusterConfig.h"

using eicrecon::CalorimeterIslandCluster;
using eicrecon::CalorimeterIslandClusterConfig;

TEST_CASE( "the clustering algorithm runs", "[CalorimeterIslandCluster]" ) {
  CalorimeterIslandCluster algo("CalorimeterIslandCluster");

  std::shared_ptr<spdlog::logger> logger = spdlog::default_logger()->clone("CalorimeterIslandCluster");
  logger->set_level(spdlog::level::trace);

  CalorimeterIslandClusterConfig cfg;
  cfg.minClusterHitEdep = 0. * dd4hep::GeV;
  cfg.minClusterCenterEdep = 0. * dd4hep::GeV;

  auto detector = dd4hep::Detector::make_unique("");
  dd4hep::Readout readout(std::string("MockCalorimeterHits"));
  dd4hep::IDDescriptor id_desc("MockCalorimeterHits", "system:8,x:8,y:8");
  readout.setIDDescriptor(id_desc);
  detector->add(id_desc);
  detector->add(readout);

  SECTION( "without splitting" ) {
    bool use_adjacencyMatrix = GENERATE(false, true);
    cfg.splitCluster = false;
    if (use_adjacencyMatrix) {
      cfg.adjacencyMatrix = "abs(x_1 - x_2) + abs(y_1 - y_2) == 1";
      cfg.readout = "MockCalorimeterHits";
    } else {
      cfg.localDistXY = {1 * dd4hep::mm, 1 * dd4hep::mm};
    }
    algo.applyConfig(cfg);
    algo.init(detector.get(), logger);

    SECTION( "on a single cell" ) {
      edm4eic::CalorimeterHitCollection hits_coll;
      hits_coll.create(
        id_desc.encode({{"system", 255}, {"x", 0}, {"y", 0}}), // std::uint64_t cellID,
        5.0, // float energy,
        0.0, // float energyError,
        0.0, // float time,
        0.0, // float timeError,
        edm4hep::Vector3f(0.0, 0.0, 0.0), // edm4hep::Vector3f position,
        edm4hep::Vector3f(0.0, 0.0, 0.0), // edm4hep::Vector3f dimension,
        0, // std::int32_t sector,
        0, // std::int32_t layer,
        edm4hep::Vector3f(0.0, 0.0, 0.0) // edm4hep::Vector3f local
      );
      auto protoclust_coll = std::make_unique<edm4eic::ProtoClusterCollection>();
      algo.process({&hits_coll}, {protoclust_coll.get()});

      REQUIRE( (*protoclust_coll).size() == 1 );
      REQUIRE( (*protoclust_coll)[0].hits_size() == 1 );
      REQUIRE( (*protoclust_coll)[0].weights_size() == 1 );
    }

    SECTION( "on two separated cells" ) {
      edm4eic::CalorimeterHitCollection hits_coll;
      hits_coll.create(
        0, // std::uint64_t cellID,
        5.0, // float energy,
        0.0, // float energyError,
        0.0, // float time,
        0.0, // float timeError,
        edm4hep::Vector3f(0.0, 0.0, 0.0), // edm4hep::Vector3f position,
        edm4hep::Vector3f(1.0, 1.0, 0.0), // edm4hep::Vector3f dimension,
        0, // std::int32_t sector,
        0, // std::int32_t layer,
        edm4hep::Vector3f(0.0, 0.0, 0.0) // edm4hep::Vector3f local
      );
      hits_coll.create(
        1, // std::uint64_t cellID,
        6.0, // float energy,
        0.0, // float energyError,
        0.0, // float time,
        0.0, // float timeError,
        edm4hep::Vector3f(0.0, 0.0, 0.0), // edm4hep::Vector3f position,
        edm4hep::Vector3f(1.0, 1.0, 0.0), // edm4hep::Vector3f dimension,
        0, // std::int32_t sector,
        0, // std::int32_t layer,
        edm4hep::Vector3f(1.1 /* mm */, 1.1 /* mm */, 0.0) // edm4hep::Vector3f local
      );
      auto protoclust_coll = std::make_unique<edm4eic::ProtoClusterCollection>();
      algo.process({&hits_coll}, {protoclust_coll.get()});

      REQUIRE( (*protoclust_coll).size() == 2 );
      REQUIRE( (*protoclust_coll)[0].hits_size() == 1 );
      REQUIRE( (*protoclust_coll)[0].weights_size() == 1 );
      REQUIRE( (*protoclust_coll)[1].hits_size() == 1 );
      REQUIRE( (*protoclust_coll)[1].weights_size() == 1 );
    }

    SECTION( "on two adjacent cells" ) {
      edm4eic::CalorimeterHitCollection hits_coll;
      hits_coll.create(
        id_desc.encode({{"system", 255}, {"x", 0}, {"y", 0}}), // std::uint64_t cellID,
        5.0, // float energy,
        0.0, // float energyError,
        0.0, // float time,
        0.0, // float timeError,
        edm4hep::Vector3f(0.0, 0.0, 0.0), // edm4hep::Vector3f position,
        edm4hep::Vector3f(1.0, 1.0, 0.0), // edm4hep::Vector3f dimension,
        0, // std::int32_t sector,
        0, // std::int32_t layer,
        edm4hep::Vector3f(0.0, 0.0, 0.0) // edm4hep::Vector3f local
      );
      hits_coll.create(
        id_desc.encode({{"system", 255}, {"x", 1}, {"y", 0}}), // std::uint64_t cellID,
        6.0, // float energy,
        0.0, // float energyError,
        0.0, // float time,
        0.0, // float timeError,
        edm4hep::Vector3f(0.0, 0.0, 0.0), // edm4hep::Vector3f position,
        edm4hep::Vector3f(1.0, 1.0, 0.0), // edm4hep::Vector3f dimension,
        0, // std::int32_t sector,
        0, // std::int32_t layer,
        edm4hep::Vector3f(0.9 /* mm */, 0.9 /* mm */, 0.0) // edm4hep::Vector3f local
      );
      auto protoclust_coll = std::make_unique<edm4eic::ProtoClusterCollection>();
      algo.process({&hits_coll}, {protoclust_coll.get()});

      REQUIRE( (*protoclust_coll).size() == 1 );
      REQUIRE( (*protoclust_coll)[0].hits_size() == 2 );
      REQUIRE( (*protoclust_coll)[0].weights_size() == 2 );
    }
  }

  SECTION( "run on three adjacent cells" ) {
    bool use_adjacencyMatrix = GENERATE(false, true);
    if (use_adjacencyMatrix) {
      cfg.adjacencyMatrix = "abs(x_1 - x_2) + abs(y_1 - y_2) == 1";
      cfg.readout = "MockCalorimeterHits";
    } else {
      cfg.localDistXY = {1 * dd4hep::mm, 1 * dd4hep::mm};
    }

    cfg.splitCluster = GENERATE(false, true);
    if (cfg.splitCluster) {
      cfg.transverseEnergyProfileMetric = "localDistXY";
      cfg.transverseEnergyProfileScale = std::numeric_limits<decltype(cfg.transverseEnergyProfileScale)>::infinity();
    }
    cfg.localDistXY = {1 * dd4hep::mm, 1 * dd4hep::mm};
    algo.applyConfig(cfg);
    algo.init(detector.get(), logger);

    edm4eic::CalorimeterHitCollection hits_coll;
    hits_coll.create(
      id_desc.encode({{"system", 255}, {"x", 0}, {"y", 0}}), // std::uint64_t cellID,
      5.0, // float energy,
      0.0, // float energyError,
      0.0, // float time,
      0.0, // float timeError,
      edm4hep::Vector3f(0.0, 0.0, 0.0), // edm4hep::Vector3f position,
      edm4hep::Vector3f(1.0, 1.0, 0.0), // edm4hep::Vector3f dimension,
      0, // std::int32_t sector,
      0, // std::int32_t layer,
      edm4hep::Vector3f(0.0, 0.0, 0.0) // edm4hep::Vector3f local
    );
    hits_coll.create(
      id_desc.encode({{"system", 255}, {"x", 1}, {"y", 0}}), // std::uint64_t cellID,
      1.0, // float energy,
      0.0, // float energyError,
      0.0, // float time,
      0.0, // float timeError,
      edm4hep::Vector3f(0.0, 0.0, 0.0), // edm4hep::Vector3f position,
      edm4hep::Vector3f(1.0, 1.0, 0.0), // edm4hep::Vector3f dimension,
      0, // std::int32_t sector,
      0, // std::int32_t layer,
      edm4hep::Vector3f(0.9 /* mm */, 0.9 /* mm */, 0.0) // edm4hep::Vector3f local
    );
    hits_coll.create(
      id_desc.encode({{"system", 255}, {"x", 2}, {"y", 0}}), // std::uint64_t cellID,
      6.0, // float energy,
      0.0, // float energyError,
      0.0, // float time,
      0.0, // float timeError,
      edm4hep::Vector3f(0.0, 0.0, 0.0), // edm4hep::Vector3f position,
      edm4hep::Vector3f(1.0, 1.0, 0.0), // edm4hep::Vector3f dimension,
      0, // std::int32_t sector,
      0, // std::int32_t layer,
      edm4hep::Vector3f(1.8 /* mm */, 1.8 /* mm */, 0.0) // edm4hep::Vector3f local
    );
    auto protoclust_coll = std::make_unique<edm4eic::ProtoClusterCollection>();
    algo.process({&hits_coll}, {protoclust_coll.get()});

    if (cfg.splitCluster) {
      REQUIRE( (*protoclust_coll).size() == 2 );
      REQUIRE( (*protoclust_coll)[0].hits_size() == 3 );
      REQUIRE( (*protoclust_coll)[0].weights_size() == 3 );
      for (double weight : (*protoclust_coll)[0].getWeights()) {
        double energy_fraction = hits_coll[0].getEnergy() / (hits_coll[0].getEnergy() + hits_coll[2].getEnergy());
        REQUIRE_THAT( weight, Catch::Matchers::WithinAbs(energy_fraction, 1e-5) );
      }
      REQUIRE( (*protoclust_coll)[1].hits_size() == 3 );
      REQUIRE( (*protoclust_coll)[1].weights_size() == 3 );
      for (double weight : (*protoclust_coll)[1].getWeights()) {
        double energy_fraction = hits_coll[2].getEnergy() / (hits_coll[0].getEnergy() + hits_coll[2].getEnergy());
        REQUIRE_THAT( weight, Catch::Matchers::WithinAbs(energy_fraction, 1e-5) );
      }
    } else {
      REQUIRE( (*protoclust_coll).size() == 1 );
      REQUIRE( (*protoclust_coll)[0].hits_size() == 3 );
      REQUIRE( (*protoclust_coll)[0].weights_size() == 3 );
    }
  }
}
