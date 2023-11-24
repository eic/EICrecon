// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Dmitry Kalinkin

#include <DD4hep/Detector.h>
#include <Evaluator/DD4hepUnits.h>
#include <catch2/catch_test_macros.hpp>
#include <edm4hep/CaloHitContributionCollection.h>
#include <edm4hep/RawCalorimeterHitCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/Vector3f.h>
#include <math.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <memory>
#include <string>
#include <vector>

#include "algorithms/calorimetry/CalorimeterHitDigi.h"
#include "algorithms/calorimetry/CalorimeterHitDigiConfig.h"

using eicrecon::CalorimeterHitDigi;
using eicrecon::CalorimeterHitDigiConfig;

TEST_CASE( "the clustering algorithm runs", "[CalorimeterHitDigi]" ) {
  CalorimeterHitDigi algo("test");

  std::shared_ptr<spdlog::logger> logger = spdlog::default_logger()->clone("CalorimeterHitDigi");
  logger->set_level(spdlog::level::trace);

  auto detector = dd4hep::Detector::make_unique("");

  CalorimeterHitDigiConfig cfg;
  cfg.threshold = 0. /* GeV */;
  cfg.corrMeanScale = 1.;

  // Keep smearing parameters at zero
  cfg.pedSigmaADC = 0;
  cfg.tRes = 0. * dd4hep::ns;
  cfg.eRes = {0. * sqrt(dd4hep::GeV), 0., 0. * dd4hep::GeV};

  SECTION( "single hit with couple contributions" ) {
    cfg.capADC = 555;
    cfg.dyRangeADC = 5.0 /* GeV */;
    cfg.pedMeanADC = 123;
    cfg.resolutionTDC = 1.0 * dd4hep::ns;
    algo.applyConfig(cfg);
    algo.init(detector.get(), logger);

    auto calohits = std::make_unique<edm4hep::CaloHitContributionCollection>();
    auto simhits = std::make_unique<edm4hep::SimCalorimeterHitCollection>();
    auto mhit = simhits->create(
      0xABABABAB, // std::uint64_t cellID
      1.0 /* GeV */, // float energy
      edm4hep::Vector3f({0. /* mm */, 0. /* mm */, 0. /* mm */}) // edm4hep::Vector3f position
    );
    mhit.addToContributions(calohits->create(
      0, // std::int32_t PDG
      0.5 /* GeV */, // float energy
      7.0 /* ns */, // float time
      edm4hep::Vector3f({0. /* mm */, 0. /* mm */, 0. /* mm */}) // edm4hep::Vector3f stepPosition
    ));
    mhit.addToContributions(calohits->create(
      0, // std::int32_t PDG
      0.5 /* GeV */, // float energy
      9.0 /* ns */, // float time
      edm4hep::Vector3f({0. /* mm */, 0. /* mm */, 0. /* mm */}) // edm4hep::Vector3f stepPosition
    ));

    auto rawhits = std::make_unique<edm4hep::RawCalorimeterHitCollection>();
    algo.process({simhits.get()}, {rawhits.get()});

    REQUIRE( (*rawhits).size() == 1 );
    REQUIRE( (*rawhits)[0].getCellID() == 0xABABABAB);
    REQUIRE( (*rawhits)[0].getAmplitude() == 123 + 111 );
    REQUIRE( (*rawhits)[0].getTimeStamp() == 7 ); // currently, earliest contribution is returned
  }
}
