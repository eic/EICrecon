// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2025, Dmitry Kalinkin

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <Evaluator/DD4hepUnits.h>
#include <algorithms/geo.h>
#include <algorithms/logger.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <edm4eic/MCRecoCalorimeterHitAssociationCollection.h>
#include <edm4eic/MCRecoCalorimeterHitLinkCollection.h>
#include <edm4hep/CaloHitContributionCollection.h>
#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/RawCalorimeterHitCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/Vector3f.h>
#include <podio/detail/Link.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <cmath>
#include <deque>
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
  [[maybe_unused]] const float EPSILON = 1e-5;

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
    edm4eic::MCRecoCalorimeterHitLinkCollection rawlinks;
    algo.process({headers.get(), simhits.get()}, {rawhits.get(), &rawlinks, rawassocs.get()});

    REQUIRE((*rawhits).size() == 1);
    REQUIRE((*rawhits)[0].getCellID() == id_desc.encode({{"system", 255}, {"x", 0}, {"y", 0}}));
    REQUIRE((*rawhits)[0].getAmplitude() == 123 + 111);
    REQUIRE((*rawhits)[0].getTimeStamp() == 7); // currently, earliest contribution is returned

    REQUIRE((*rawassocs).size() == 1);
    REQUIRE((*rawassocs)[0].getSimHit() == (*simhits)[0]);
    REQUIRE((*rawassocs)[0].getRawHit() == (*rawhits)[0]);

    // Validate links collection
    REQUIRE(rawlinks.size() == 1);
    REQUIRE(rawlinks.size() == (*rawassocs).size());

    // Check link from/to relationships match association sim/raw hits
    REQUIRE(rawlinks[0].getFrom() == (*rawhits)[0]);
    REQUIRE(rawlinks[0].getTo() == (*simhits)[0]);

    // Verify weights are normalized (should be 1.0 for single hit)
    REQUIRE_THAT(rawlinks[0].getWeight(), Catch::Matchers::WithinAbs(1.0, EPSILON));
  }
}

TEST_CASE("photon counting readouts do not truncate photon counts", "[CalorimeterHitDigi]") {
  auto detector = algorithms::GeoSvc::instance().detector();
  auto id_desc  = detector->readout("MockCalorimeterHits").idSpec();

  CalorimeterHitDigi algo("test");

  CalorimeterHitDigiConfig cfg;
  cfg.threshold     = 0. /* GeV */;
  cfg.corrMeanScale = "1.";
  cfg.pedMeanADC    = 0;
  cfg.pedSigmaADC   = 0;
  cfg.tRes          = 0. * dd4hep::ns;
  cfg.eRes          = {0. * sqrt(dd4hep::GeV), 0., 0. * dd4hep::GeV};
  cfg.readout       = "MockCalorimeterHits";
  cfg.capADC        = 1000;
  cfg.dyRangeADC    = 5.0 /* GeV */;
  cfg.lightYield    = 1.0 /* 1/GeV */;

  // Average amplitude of a single hit with the given energy, over many events
  constexpr int n_events = 1000;
  auto mean_amplitude    = [&](float energy) {
    double sum = 0;
    for (int event = 0; event < n_events; ++event) {
      auto headers = std::make_unique<edm4hep::EventHeaderCollection>();
      headers->create(event, 1, 12345678, 1.0);
      auto calohits = std::make_unique<edm4hep::CaloHitContributionCollection>();
      auto simhits  = std::make_unique<edm4hep::SimCalorimeterHitCollection>();
      auto mhit     = simhits->create(id_desc.encode({{"system", 255}, {"x", 0}, {"y", 0}}), energy,
                                      edm4hep::Vector3f({0., 0., 0.}));
      mhit.addToContributions(
          calohits->create(0, energy, 7.0 /* ns */, edm4hep::Vector3f({0., 0., 0.})));

      auto rawhits   = std::make_unique<edm4hep::RawCalorimeterHitCollection>();
      auto rawassocs = std::make_unique<edm4eic::MCRecoCalorimeterHitAssociationCollection>();
      edm4eic::MCRecoCalorimeterHitLinkCollection rawlinks;
      algo.process({headers.get(), simhits.get()}, {rawhits.get(), &rawlinks, rawassocs.get()});

      REQUIRE((*rawhits).size() == 1);
      sum += (*rawhits)[0].getAmplitude();
    }
    return sum / n_events;
  };

  SECTION("poisson_photon readout with a mean below one photon") {
    cfg.readoutType               = "poisson_photon";
    cfg.photonDetectionEfficiency = 1.0;
    algo.applyConfig(cfg);
    algo.init();

    // 0.5 photons on average, 5 photons at the top of the dynamic range: 200 ADC per photon.
    // A truncated Poisson mean would always give 0 photons.
    REQUIRE_THAT(mean_amplitude(0.5 /* GeV */), Catch::Matchers::WithinAbs(0.5 * 200, 0.1 * 200));
  }

  SECTION("sipm readout with fewer produced photons than one") {
    cfg.readoutType               = "sipm";
    cfg.photonDetectionEfficiency = 0.5;
    cfg.numEffectiveSipmPixels    = 1000000;
    algo.applyConfig(cfg);
    algo.init();

    // 0.6 photons are rounded to 1 produced photon, detected with probability 0.5, and
    // 2.5 photons at the top of the dynamic range: 400 ADC per fired pixel.
    // Truncated photon or fired pixel counts would always give 0.
    REQUIRE_THAT(mean_amplitude(0.6 /* GeV */), Catch::Matchers::WithinAbs(0.5 * 400, 0.1 * 400));
  }
}
