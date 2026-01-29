// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2025, Dmitry Kalinkin

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <Evaluator/DD4hepUnits.h>
#include <algorithms/geo.h>
#include <algorithms/logger.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <edm4eic/MCRecoCalorimeterHitAssociationCollection.h>
#include <edm4eic/EDM4eicVersion.h>
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
#include <edm4eic/MCRecoCalorimeterHitLinkCollection.h>
#endif
#include <edm4hep/CaloHitContributionCollection.h>
#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/RawCalorimeterHitCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/Vector3f.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <cmath>
#include <gsl/pointers>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/calorimetry/CalorimeterHitDigi.h"
#include "algorithms/calorimetry/CalorimeterHitDigiConfig.h"

using eicrecon::CalorimeterHitDigi;
using eicrecon::CalorimeterHitDigiConfig;

TEST_CASE("the clustering algorithm runs", "[CalorimeterHitDigi]") {
  const float EPSILON = 1e-5;

  std::shared_ptr<spdlog::logger> logger = spdlog::default_logger()->clone("CalorimeterHitDigi");
  logger->set_level(spdlog::level::trace);

  auto detector = algorithms::GeoSvc::instance().detector();
  auto id_desc  = detector->readout("MockCalorimeterHits").idSpec();

  CalorimeterHitDigi algo("test");

  CalorimeterHitDigiConfig cfg;
  cfg.threshold     = 0. /* GeV */;
  cfg.corrMeanScale = "1.";

  // Keep smearing parameters at zero
  cfg.pedSigmaADC = 0;
  cfg.tRes        = 0. * dd4hep::ns;
  cfg.eRes        = {0. * sqrt(dd4hep::GeV), 0., 0. * dd4hep::GeV};
  cfg.readout     = "MockCalorimeterHits";

  SECTION("single hit with couple contributions") {
    cfg.capADC        = 555;
    cfg.dyRangeADC    = 5.0 /* GeV */;
    cfg.pedMeanADC    = 123;
    cfg.resolutionTDC = 1.0 * dd4hep::ns;
    algo.level(algorithms::LogLevel(spdlog::level::trace));
    algo.applyConfig(cfg);
    algo.init();

    auto headers = std::make_unique<edm4hep::EventHeaderCollection>();
    auto header  = headers->create(1, 1, 12345678, 1.0);

    auto calohits = std::make_unique<edm4hep::CaloHitContributionCollection>();
    auto simhits  = std::make_unique<edm4hep::SimCalorimeterHitCollection>();
    auto mhit     = simhits->create(
        id_desc.encode({{"system", 255}, {"x", 0}, {"y", 0}}),     // std::uint64_t cellID,
        1.0 /* GeV */,                                             // float energy
        edm4hep::Vector3f({0. /* mm */, 0. /* mm */, 0. /* mm */}) // edm4hep::Vector3f position
    );
    mhit.addToContributions(calohits->create(
        0,                                                         // std::int32_t PDG
        0.5 /* GeV */,                                             // float energy
        7.0 /* ns */,                                              // float time
        edm4hep::Vector3f({0. /* mm */, 0. /* mm */, 0. /* mm */}) // edm4hep::Vector3f stepPosition
        ));
    mhit.addToContributions(calohits->create(
        0,                                                         // std::int32_t PDG
        0.5 /* GeV */,                                             // float energy
        9.0 /* ns */,                                              // float time
        edm4hep::Vector3f({0. /* mm */, 0. /* mm */, 0. /* mm */}) // edm4hep::Vector3f stepPosition
        ));

    auto rawhits   = std::make_unique<edm4hep::RawCalorimeterHitCollection>();
    auto rawassocs = std::make_unique<edm4eic::MCRecoCalorimeterHitAssociationCollection>();
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
    edm4eic::MCRecoCalorimeterHitLinkCollection rawlinks;
    algo.process({headers.get(), simhits.get()}, {rawhits.get(), &rawlinks, rawassocs.get()});
#else
    algo.process({headers.get(), simhits.get()}, {rawhits.get(), rawassocs.get()});
#endif

    REQUIRE((*rawhits).size() == 1);
    REQUIRE((*rawhits)[0].getCellID() == id_desc.encode({{"system", 255}, {"x", 0}, {"y", 0}}));
    REQUIRE((*rawhits)[0].getAmplitude() == 123 + 111);
    REQUIRE((*rawhits)[0].getTimeStamp() == 7); // currently, earliest contribution is returned

    REQUIRE((*rawassocs).size() == 1);
    REQUIRE((*rawassocs)[0].getSimHit() == (*simhits)[0]);
    REQUIRE((*rawassocs)[0].getRawHit() == (*rawhits)[0]);

#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
    // Validate links collection
    REQUIRE(rawlinks.size() == 1);
    REQUIRE(rawlinks.size() == (*rawassocs).size());

    // Check link from/to relationships match association sim/raw hits
    REQUIRE(rawlinks[0].getFrom() == (*rawhits)[0]);
    REQUIRE(rawlinks[0].getTo() == (*simhits)[0]);

    // Verify weights are normalized (should be 1.0 for single hit)
    REQUIRE_THAT(rawlinks[0].getWeight(), Catch::Matchers::WithinAbs(1.0, EPSILON));
#endif
  }
}
