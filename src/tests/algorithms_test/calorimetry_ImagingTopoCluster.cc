// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Sebouh Paul

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <Evaluator/DD4hepUnits.h>
#include <algorithms/geo.h>
#include <catch2/catch_test_macros.hpp>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/ProtoClusterCollection.h>
#include <edm4hep/Vector3f.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <gsl/pointers>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "algorithms/calorimetry/ImagingTopoCluster.h"
#include "algorithms/calorimetry/ImagingTopoClusterConfig.h"

using eicrecon::ImagingTopoCluster;
using eicrecon::ImagingTopoClusterConfig;

TEST_CASE("the clustering algorithm runs", "[ImagingTopoCluster]") {
  ImagingTopoCluster algo("ImagingTopoCluster");

  std::shared_ptr<spdlog::logger> logger = spdlog::default_logger()->clone("ImagingTopoCluster");
  logger->set_level(spdlog::level::trace);

  ImagingTopoClusterConfig cfg;
  cfg.layerMode            = eicrecon::ImagingTopoClusterConfig::ELayerMode::xy;
  cfg.minClusterHitEdep    = 0. * dd4hep::GeV;
  cfg.minClusterCenterEdep = 0. * dd4hep::GeV;
  cfg.localDistXY          = {1.0 * dd4hep::mm, 1.0 * dd4hep::mm}; //mm
  cfg.layerDistXY          = {1.0 * dd4hep::mm, 1.0 * dd4hep::mm}; //mm
  cfg.minClusterEdep       = 9 * dd4hep::MeV;
  // minimum number of hits (to save this cluster)
  cfg.minClusterNhits = 1;
  auto detector       = algorithms::GeoSvc::instance().detector();
  auto id_desc        = detector->readout("MockCalorimeterHits").idSpec();

  SECTION("without splitting") {
    algo.applyConfig(cfg);
    algo.init();

    SECTION("on a single cell") {
      edm4eic::CalorimeterHitCollection hits_coll;
      hits_coll.create(
          id_desc.encode(
              {{"system", 255}, {"x", 0}, {"y", 0}, {"layer", 0}}), // std::uint64_t cellID,
          5.0,                                                      // float energy,
          0.0,                                                      // float energyError,
          0.0,                                                      // float time,
          0.0,                                                      // float timeError,
          edm4hep::Vector3f(0.0, 0.0, 0.0),                         // edm4hep::Vector3f position,
          edm4hep::Vector3f(0.0, 0.0, 0.0),                         // edm4hep::Vector3f dimension,
          0,                                                        // std::int32_t sector,
          0,                                                        // std::int32_t layer,
          edm4hep::Vector3f(0.0, 0.0, 0.0)                          // edm4hep::Vector3f local
      );
      auto protoclust_coll = std::make_unique<edm4eic::ProtoClusterCollection>();
      algo.process({&hits_coll}, {protoclust_coll.get()});

      REQUIRE((*protoclust_coll).size() == 1);
      REQUIRE((*protoclust_coll)[0].hits_size() == 1);
      REQUIRE((*protoclust_coll)[0].weights_size() == 1);
    }

    SECTION("on two separated cells") {
      edm4eic::CalorimeterHitCollection hits_coll;
      hits_coll.create(
          id_desc.encode(
              {{"system", 255}, {"x", 0}, {"y", 0}, {"layer", 0}}), // std::uint64_t cellID,
          5.0,                                                      // float energy,
          0.0,                                                      // float energyError,
          0.0,                                                      // float time,
          0.0,                                                      // float timeError,
          edm4hep::Vector3f(0.0, 0.0, 0.0),                         // edm4hep::Vector3f position,
          edm4hep::Vector3f(1.0, 1.0, 0.0),                         // edm4hep::Vector3f dimension,
          0,                                                        // std::int32_t sector,
          0,                                                        // std::int32_t layer,
          edm4hep::Vector3f(0.0, 0.0, 0.0)                          // edm4hep::Vector3f local
      );
      hits_coll.create(
          id_desc.encode(
              {{"system", 255}, {"x", 2}, {"y", 2}, {"layer", 0}}), // std::uint64_t cellID,
          6.0,                                                      // float energy,
          0.0,                                                      // float energyError,
          0.0,                                                      // float time,
          0.0,                                                      // float timeError,
          edm4hep::Vector3f(1.1, 1.1, 0.0),                         // edm4hep::Vector3f position,
          edm4hep::Vector3f(1.0, 1.0, 0.0),                         // edm4hep::Vector3f dimension,
          0,                                                        // std::int32_t sector,
          0,                                                        // std::int32_t layer,
          edm4hep::Vector3f(1.1 /* mm */, 1.1 /* mm */, 0.0)        // edm4hep::Vector3f local
      );
      auto protoclust_coll = std::make_unique<edm4eic::ProtoClusterCollection>();
      algo.process({&hits_coll}, {protoclust_coll.get()});

      REQUIRE((*protoclust_coll).size() == 2);
      REQUIRE((*protoclust_coll)[0].hits_size() == 1);
      REQUIRE((*protoclust_coll)[0].weights_size() == 1);
      REQUIRE((*protoclust_coll)[1].hits_size() == 1);
      REQUIRE((*protoclust_coll)[1].weights_size() == 1);
    }

    SECTION("on two adjacent cells (same layer)") {
      edm4eic::CalorimeterHitCollection hits_coll;
      hits_coll.create(
          id_desc.encode({{"system", 255}, {"x", 0}, {"y", 0}}), // std::uint64_t cellID,
          5.0,                                                   // float energy,
          0.0,                                                   // float energyError,
          0.0,                                                   // float time,
          0.0,                                                   // float timeError,
          edm4hep::Vector3f(0.0, 0.0, 0.0),                      // edm4hep::Vector3f position,
          edm4hep::Vector3f(1.0, 1.0, 0.0),                      // edm4hep::Vector3f dimension,
          0,                                                     // std::int32_t sector,
          0,                                                     // std::int32_t layer,
          edm4hep::Vector3f(0.0, 0.0, 0.0)                       // edm4hep::Vector3f local
      );
      hits_coll.create(
          id_desc.encode({{"system", 255}, {"x", 1}, {"y", 0}}), // std::uint64_t cellID,
          6.0,                                                   // float energy,
          0.0,                                                   // float energyError,
          0.0,                                                   // float time,
          0.0,                                                   // float timeError,
          edm4hep::Vector3f(0.9, 0.9, 0.0),                      // edm4hep::Vector3f position,
          edm4hep::Vector3f(1.0, 1.0, 0.0),                      // edm4hep::Vector3f dimension,
          0,                                                     // std::int32_t sector,
          0,                                                     // std::int32_t layer,
          edm4hep::Vector3f(0.9 /* mm */, 0.9 /* mm */, 0.0)     // edm4hep::Vector3f local
      );
      auto protoclust_coll = std::make_unique<edm4eic::ProtoClusterCollection>();
      algo.process({&hits_coll}, {protoclust_coll.get()});

      REQUIRE((*protoclust_coll).size() == 1);
      REQUIRE((*protoclust_coll)[0].hits_size() == 2);
      REQUIRE((*protoclust_coll)[0].weights_size() == 2);
    }
  }

  SECTION("run on three cells, two of which are on the same layer, and there is a third one on "
          "another layer acting as a bridge between them") {

    cfg.localDistXY = {1 * dd4hep::mm, 1 * dd4hep::mm};
    algo.applyConfig(cfg);
    algo.init();

    edm4eic::CalorimeterHitCollection hits_coll;
    hits_coll.create(
        id_desc.encode(
            {{"system", 255}, {"x", 0}, {"y", 0}, {"layer", 0}}), // std::uint64_t cellID,
        5.0,                                                      // float energy,
        0.0,                                                      // float energyError,
        0.0,                                                      // float time,
        0.0,                                                      // float timeError,
        edm4hep::Vector3f(0.0, 0.0, 0.0),                         // edm4hep::Vector3f position,
        edm4hep::Vector3f(1.0, 1.0, 0.0),                         // edm4hep::Vector3f dimension,
        0,                                                        // std::int32_t sector,
        0,                                                        // std::int32_t layer,
        edm4hep::Vector3f(0.0, 0.0, 0.0)                          // edm4hep::Vector3f local
    );
    hits_coll.create(
        id_desc.encode(
            {{"system", 255}, {"x", 1}, {"y", 0}, {"layer", 1}}), // std::uint64_t cellID,
        1.0,                                                      // float energy,
        0.0,                                                      // float energyError,
        0.0,                                                      // float time,
        0.0,                                                      // float timeError,
        edm4hep::Vector3f(0.9, 0.9, 0.0),                         // edm4hep::Vector3f position,
        edm4hep::Vector3f(1.0, 1.0, 0.0),                         // edm4hep::Vector3f dimension,
        0,                                                        // std::int32_t sector,
        1,                                                        // std::int32_t layer,
        edm4hep::Vector3f(0.9 /* mm */, 0.9 /* mm */, 0.0)        // edm4hep::Vector3f local
    );
    hits_coll.create(
        id_desc.encode(
            {{"system", 255}, {"x", 2}, {"y", 0}, {"layer", 0}}), // std::uint64_t cellID,
        6.0,                                                      // float energy,
        0.0,                                                      // float energyError,
        0.0,                                                      // float time,
        0.0,                                                      // float timeError,
        edm4hep::Vector3f(1.8, 1.8, 0.0),                         // edm4hep::Vector3f position,
        edm4hep::Vector3f(1.0, 1.0, 0.0),                         // edm4hep::Vector3f dimension,
        0,                                                        // std::int32_t sector,
        0,                                                        // std::int32_t layer,
        edm4hep::Vector3f(1.8 /* mm */, 1.8 /* mm */, 0.0)        // edm4hep::Vector3f local
    );
    auto protoclust_coll = std::make_unique<edm4eic::ProtoClusterCollection>();
    algo.process({&hits_coll}, {protoclust_coll.get()});

    REQUIRE((*protoclust_coll).size() == 1);
    REQUIRE((*protoclust_coll)[0].hits_size() == 3);
    REQUIRE((*protoclust_coll)[0].weights_size() == 3);
  }
}
