// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Dmitry Kalinkin

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <Evaluator/DD4hepUnits.h>
#include <algorithms/geo.h>
#include <algorithms/logger.h>
#include <catch2/catch_test_macros.hpp>
#include <edm4eic/EDM4eicVersion.h>
#if EDM4EIC_VERSION_MAJOR >= 7
#include <edm4eic/MCRecoCalorimeterHitAssociationCollection.h>
#endif
#include <edm4hep/CaloHitContributionCollection.h>
#include <edm4hep/RawCalorimeterHitCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/Vector3f.h>
#include <math.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <gsl/pointers>
#include <memory>
#include <utility>
#include <vector>

#include "algorithms/calorimetry/CalorimeterHitDigi.h"
#include "algorithms/calorimetry/CalorimeterHitDigiConfig.h"

using eicrecon::CalorimeterHitDigi;
using eicrecon::CalorimeterHitDigiConfig;

TEST_CASE( "the clustering algorithm runs", "[CalorimeterHitDigi]" ) {
  std::shared_ptr<spdlog::logger> logger = spdlog::default_logger()->clone("CalorimeterHitDigi");
  logger->set_level(spdlog::level::trace);

  auto detector = algorithms::GeoSvc::instance().detector();
  auto id_desc = detector->readout("MockCalorimeterHits").idSpec();

  CalorimeterHitDigi algo("test");

  CalorimeterHitDigiConfig cfg;
  cfg.threshold = 0. /* GeV */;
  cfg.corrMeanScale = "1.";

  // Keep smearing parameters at zero
  cfg.pedSigmaADC = 0;
  cfg.tRes = 0. * dd4hep::ns;
  cfg.eRes = {0. * sqrt(dd4hep::GeV), 0., 0. * dd4hep::GeV};
  cfg.readout = "MockCalorimeterHits";

  SECTION( "single hit with couple contributions" ) {
    cfg.capADC = 555;
    cfg.dyRangeADC = 5.0 /* GeV */;
    cfg.pedMeanADC = 123;
    cfg.resolutionTDC = 1.0 * dd4hep::ns;
    algo.level(algorithms::LogLevel(spdlog::level::trace));
    algo.applyConfig(cfg);
    algo.init();

    auto calohits = std::make_unique<edm4hep::CaloHitContributionCollection>();
    auto simhits = std::make_unique<edm4hep::SimCalorimeterHitCollection>();
    auto mhit = simhits->create(
      id_desc.encode({{"system", 255}, {"x", 0}, {"y", 0}}), // std::uint64_t cellID,
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
#if EDM4EIC_VERSION_MAJOR >= 7
    auto rawassocs = std::make_unique<edm4eic::MCRecoCalorimeterHitAssociationCollection>();
    algo.process({simhits.get()}, {rawhits.get(), rawassocs.get()});
#else
    algo.process({simhits.get()}, {rawhits.get()});
#endif

    REQUIRE( (*rawhits).size() == 1 );
    REQUIRE( (*rawhits)[0].getCellID() == id_desc.encode({{"system", 255}, {"x", 0}, {"y", 0}}));
    REQUIRE( (*rawhits)[0].getAmplitude() == 123 + 111 );
    REQUIRE( (*rawhits)[0].getTimeStamp() == 7 ); // currently, earliest contribution is returned

#if EDM4EIC_VERSION_MAJOR >= 7
    REQUIRE( (*rawassocs).size() == 1 );
    REQUIRE( (*rawassocs)[0].getSimHit() == (*simhits)[0] );
    REQUIRE( (*rawassocs)[0].getRawHit() == (*rawhits)[0] );
#endif
  }
}
