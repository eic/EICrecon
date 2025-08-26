// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Sebouh Paul

#include <DD4hep/Detector.h>       // for Detector
#include <DD4hep/IDDescriptor.h>   // for IDDescriptor
#include <DD4hep/Readout.h>        // for Readout
#include <Evaluator/DD4hepUnits.h> // for MeV, mm, keV, ns
#include <algorithms/geo.h>
#include <catch2/catch_test_macros.hpp> // for AssertionHandler, operator""_catch_sr, StringRef, REQUIRE, operator<, operator==, operator>, TEST_CASE
#include <edm4eic/CalorimeterHitCollection.h> // for CalorimeterHitCollection, MutableCalorimeterHit, CalorimeterHitMutableCollectionIterator
#include <edm4hep/Vector3f.h> // for Vector3f
#include <spdlog/common.h>    // for level_enum
#include <spdlog/logger.h>    // for logger
#include <spdlog/spdlog.h>    // for default_logger
#include <array>              // for array
#include <cmath>              // for sqrt, abs
#include <cstddef>
#include <gsl/pointers>
#include <memory> // for allocator, unique_ptr, make_unique, shared_ptr, __shared_ptr_access
#include <string>
#include <utility> // for pair
#include <vector>

#include "algorithms/calorimetry/HEXPLIT.h"       // for HEXPLIT
#include "algorithms/calorimetry/HEXPLITConfig.h" // for HEXPLITConfig

using eicrecon::HEXPLIT;
using eicrecon::HEXPLITConfig;

TEST_CASE("the subcell-splitting algorithm runs", "[HEXPLIT]") {
  HEXPLIT algo("HEXPLIT");

  std::shared_ptr<spdlog::logger> logger = spdlog::default_logger()->clone("HEXPLIT");
  logger->set_level(spdlog::level::trace);

  HEXPLITConfig cfg;
  cfg.MIP  = 472. * dd4hep::keV;
  cfg.tmax = 1000. * dd4hep::ns;

  auto detector = algorithms::GeoSvc::instance().detector();
  auto id_desc  = detector->readout("MockCalorimeterHits").idSpec();

  //create a geometry for the fake detector.
  double side_length   = 31.3 * dd4hep::mm;
  double layer_spacing = 25.1 * dd4hep::mm;
  double thickness     = 3 * dd4hep::mm;

  //dimension of a cell
  auto dimension = edm4hep::Vector3f(2 * side_length, sqrt(3) * side_length, thickness);

  algo.applyConfig(cfg);
  algo.init();

  edm4eic::CalorimeterHitCollection hits_coll;

  //create a set of 5 hits in consecutive layers, all of which overlap in a single rhombus,
  // centered at (3/8, sqrt(3)/8)*side_length
  std::array<double, 5> layer = {0, 1, 2, 3, 4};
  std::array<double, 5> x     = {0, 0.75 * side_length, 0, 0.75 * side_length, 0};
  std::array<double, 5> y     = {sqrt(3) / 2 * side_length, -0.25 * sqrt(3) * side_length, 0,
                                 0.25 * sqrt(3) * side_length, sqrt(3) / 2 * side_length};
  std::array<double, 5> E = {50 * dd4hep::MeV, 50 * dd4hep::MeV, 50 * dd4hep::MeV, 50 * dd4hep::MeV,
                             50 * dd4hep::MeV};
  for (std::size_t i = 0; i < 5; i++) {
    hits_coll.create(
        id_desc.encode({{"system", 255}, {"x", 0}, {"y", 0}}),   // std::uint64_t cellID,
        E[i],                                                    // float energy,
        0.0,                                                     // float energyError,
        0.0,                                                     // float time,
        0.0,                                                     // float timeError,
        edm4hep::Vector3f(x[i], y[i], layer[i] * layer_spacing), // edm4hep::Vector3f position,
        dimension,                                               // edm4hep::Vector3f dimension,
        0,                                                       // std::int32_t sector,
        layer[i],                                                // std::int32_t layer,
        edm4hep::Vector3f(x[i], y[i], layer[i] * layer_spacing)  // edm4hep::Vector3f local
    );
  }

  auto subcellhits_coll = std::make_unique<edm4eic::CalorimeterHitCollection>();
  algo.process({&hits_coll}, {subcellhits_coll.get()});

  //the number of subcell hits should be equal to the
  //number of subcells per cell (12) times the number of cells (5)
  REQUIRE((*subcellhits_coll).size() == 60);

  //next check that the sum of the hit energies equals the energy that I gave the hits
  double tol  = 0.001;
  double Esum = 0;
  int i       = 0;
  for (auto subcell : *subcellhits_coll) {
    Esum += subcell.getEnergy();
    i++;
    if (i % 12 == 0) {
      REQUIRE(std::abs(Esum - E[i / 12 - 1]) / E[i / 12 - 1] < tol);
      Esum = 0;
    }
  }
  // next check that almost all of the energy of the hit in the middle layer
  // is in the subcell where the other hits overlap
  REQUIRE((*subcellhits_coll)[35].getEnergy() / E[2] > 0.95);
}
