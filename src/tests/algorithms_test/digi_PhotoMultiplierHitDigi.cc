// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Wouter Deconinck

#include <Evaluator/DD4hepUnits.h>
#include <algorithms/logger.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/MCRecoTrackerHitLinkCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <podio/detail/Link.h>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

#include "algorithms/digi/PhotoMultiplierHitDigi.h"
#include "algorithms/digi/PhotoMultiplierHitDigiConfig.h"

using eicrecon::PhotoMultiplierHitDigi;
using eicrecon::PhotoMultiplierHitDigiConfig;

namespace {

// configuration without any random smearing or photon losses
PhotoMultiplierHitDigiConfig noSmearingConfig() {
  PhotoMultiplierHitDigiConfig cfg;
  cfg.hitTimeWindow           = 20.0;     // [ns]
  cfg.timeResolution          = 1 / 16.0; // [ns]
  cfg.speMean                 = 80.0;
  cfg.speError                = 0.0;
  cfg.pedMean                 = 200.0;
  cfg.pedError                = 0.0;
  cfg.enableNoise             = false;
  cfg.enablePixelGaps         = false;
  cfg.safetyFactor            = 1.0;
  cfg.enableQuantumEfficiency = false;
  return cfg;
}

struct DigiOutput {
  std::unique_ptr<edm4eic::RawTrackerHitCollection> raw_hits =
      std::make_unique<edm4eic::RawTrackerHitCollection>();
  std::unique_ptr<edm4eic::MCRecoTrackerHitLinkCollection> links =
      std::make_unique<edm4eic::MCRecoTrackerHitLinkCollection>();
  std::unique_ptr<edm4eic::MCRecoTrackerHitAssociationCollection> assocs =
      std::make_unique<edm4eic::MCRecoTrackerHitAssociationCollection>();
};

DigiOutput run(const PhotoMultiplierHitDigi& algo, const edm4hep::EventHeaderCollection& headers,
               const edm4hep::SimTrackerHitCollection& sim_hits) {
  DigiOutput out;
  algo.process({&headers, &sim_hits}, {out.raw_hits.get(), out.links.get(), out.assocs.get()});
  return out;
}

void addPhoton(edm4hep::SimTrackerHitCollection& sim_hits, std::uint64_t cellID, float time) {
  auto hit = sim_hits.create();
  hit.setCellID(cellID);
  hit.setEDep(3e-9 /* GeV */);
  hit.setTime(time /* ns */);
}

} // namespace

TEST_CASE("PhotoMultiplierHitDigi groups photons in a pixel", "[PhotoMultiplierHitDigi]") {
  PhotoMultiplierHitDigi algo("PhotoMultiplierHitDigi");
  algo.level(algorithms::LogLevel::kTrace);
  algo.applyConfig(noSmearingConfig());
  algo.init();

  auto headers = std::make_unique<edm4hep::EventHeaderCollection>();
  headers->create(1, 1, 12345678, 1.0);

  SECTION("single photon") {
    edm4hep::SimTrackerHitCollection sim_hits;
    addPhoton(sim_hits, 0x1, 1.0);
    auto out = run(algo, *headers, sim_hits);
    REQUIRE(out.raw_hits->size() == 1);
    CHECK((*out.raw_hits)[0].getCellID() == 0x1);
    CHECK((*out.raw_hits)[0].getCharge() == 200 + 80);
    CHECK((*out.raw_hits)[0].getTimeStamp() == 16);
    REQUIRE(out.links->size() == 1);
    CHECK((*out.links)[0].getTo() == sim_hits[0]);
    CHECK_THAT((*out.links)[0].getWeight(), Catch::Matchers::WithinAbs(1.0, 1e-6));
  }

  SECTION("photons within the time window form one hit at the earliest time") {
    // insert the later photon first: the result must not depend on the input order
    edm4hep::SimTrackerHitCollection sim_hits;
    addPhoton(sim_hits, 0x1, 11.0);
    addPhoton(sim_hits, 0x1, 1.0);
    auto out = run(algo, *headers, sim_hits);
    REQUIRE(out.raw_hits->size() == 1);
    CHECK((*out.raw_hits)[0].getCharge() == 200 + 2 * 80);
    CHECK((*out.raw_hits)[0].getTimeStamp() == 16);
    REQUIRE(out.links->size() == 2);
    for (const auto& link : *out.links) {
      CHECK_THAT(link.getWeight(), Catch::Matchers::WithinAbs(0.5, 1e-6));
    }
  }

  SECTION("two interactions separated by more than the time window give two hits") {
    const float dt = 500.0; // [ns]
    edm4hep::SimTrackerHitCollection sim_hits;
    addPhoton(sim_hits, 0x1, 1.0);
    addPhoton(sim_hits, 0x1, 1.0 + dt);
    addPhoton(sim_hits, 0x2, 1.0 + dt);
    auto out = run(algo, *headers, sim_hits);
    REQUIRE(out.raw_hits->size() == 3);
    // output is ordered by cellID, then time
    CHECK((*out.raw_hits)[0].getCellID() == 0x1);
    CHECK((*out.raw_hits)[0].getTimeStamp() == 16);
    CHECK((*out.raw_hits)[1].getCellID() == 0x1);
    CHECK((*out.raw_hits)[1].getTimeStamp() == static_cast<std::int32_t>(16 * (1.0 + dt)));
    CHECK((*out.raw_hits)[2].getCellID() == 0x2);
    for (const auto& raw_hit : *out.raw_hits) {
      CHECK(raw_hit.getCharge() == 200 + 80);
    }
    // each raw hit is linked to its own photon only
    REQUIRE(out.links->size() == 3);
    CHECK((*out.links)[0].getFrom() == (*out.raw_hits)[0]);
    CHECK((*out.links)[0].getTo() == sim_hits[0]);
    CHECK((*out.links)[1].getFrom() == (*out.raw_hits)[1]);
    CHECK((*out.links)[1].getTo() == sim_hits[1]);
    CHECK((*out.links)[2].getFrom() == (*out.raw_hits)[2]);
    CHECK((*out.links)[2].getTo() == sim_hits[2]);
  }

  SECTION("negative times are binned with floor") {
    edm4hep::SimTrackerHitCollection sim_hits;
    addPhoton(sim_hits, 0x1, -0.01);
    auto out = run(algo, *headers, sim_hits);
    REQUIRE(out.raw_hits->size() == 1);
    CHECK((*out.raw_hits)[0].getTimeStamp() == -1);
  }
}

TEST_CASE("PhotoMultiplierHitDigi noise hits", "[PhotoMultiplierHitDigi]") {
  const std::uint64_t num_pixels = 1000;
  const double mean_per_pixel    = 0.5;

  auto cfg            = noSmearingConfig();
  cfg.enableNoise     = true;
  cfg.noiseTimeWindow = 100 * dd4hep::ns;
  cfg.noiseRate       = mean_per_pixel / cfg.noiseTimeWindow;
  cfg.hitTimeWindow   = 0.0; // keep noise hits separate, so they can be counted

  PhotoMultiplierHitDigi algo("PhotoMultiplierHitDigi");
  algo.applyConfig(cfg);
  algo.SetNoisePixels(num_pixels, [](std::uint64_t index) { return 0x1000 + index; });
  algo.init();

  auto headers = std::make_unique<edm4hep::EventHeaderCollection>();
  headers->create(1, 1, 12345678, 1.0);
  edm4hep::SimTrackerHitCollection sim_hits;

  auto out = run(algo, *headers, sim_hits);

  // Poisson-distributed count, within 5 sigma of the mean
  const double mean = mean_per_pixel * num_pixels;
  CHECK(std::abs(static_cast<double>(out.raw_hits->size()) - mean) < 5 * std::sqrt(mean));
  // noise hits are on the provided pixels, inside the noise time window, without truth links
  for (const auto& raw_hit : *out.raw_hits) {
    CHECK(raw_hit.getCellID() >= 0x1000);
    CHECK(raw_hit.getCellID() < 0x1000 + num_pixels);
    CHECK(raw_hit.getTimeStamp() >= 0);
    CHECK(raw_hit.getTimeStamp() < 100 * 16);
  }
  CHECK(out.links->empty());

  SECTION("noise is reproducible for the same event") {
    auto out2 = run(algo, *headers, sim_hits);
    REQUIRE(out2.raw_hits->size() == out.raw_hits->size());
    for (std::size_t i = 0; i < out.raw_hits->size(); ++i) {
      CHECK((*out2.raw_hits)[i].getCellID() == (*out.raw_hits)[i].getCellID());
      CHECK((*out2.raw_hits)[i].getTimeStamp() == (*out.raw_hits)[i].getTimeStamp());
    }
  }

  SECTION("noise differs between events") {
    auto headers2 = std::make_unique<edm4hep::EventHeaderCollection>();
    headers2->create(2, 1, 12345678, 1.0);
    auto out2      = run(algo, *headers2, sim_hits);
    bool identical = out2.raw_hits->size() == out.raw_hits->size();
    for (std::size_t i = 0; identical && i < out.raw_hits->size(); ++i) {
      identical = (*out2.raw_hits)[i].getCellID() == (*out.raw_hits)[i].getCellID() &&
                  (*out2.raw_hits)[i].getTimeStamp() == (*out.raw_hits)[i].getTimeStamp();
    }
    CHECK_FALSE(identical);
  }
}
