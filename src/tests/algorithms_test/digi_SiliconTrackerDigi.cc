// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ePIC Collaboration

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/MCRecoTrackerHitLinkCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <cstdint>

#include "algorithms/digi/SiliconTrackerDigi.h"
#include "algorithms/digi/SiliconTrackerDigiConfig.h"

using Catch::Approx;
using eicrecon::SiliconTrackerDigi;
using eicrecon::SiliconTrackerDigiConfig;

namespace {

edm4hep::MutableSimTrackerHit addHit(edm4hep::SimTrackerHitCollection& hits,
                                    edm4hep::MCParticleCollection& particles,
                                    std::uint64_t cellID, double eDep, double time) {
  auto particle = particles.create();
  particle.setPDG(2212);
  particle.setCharge(1.0f);
  particle.setMass(0.938f);
  particle.setGeneratorStatus(1);
  particle.setTime(0.0f);

  auto hit = hits.create();
  hit.setCellID(cellID);
  hit.setPosition({0.0, 0.0, 0.0});
  hit.setMomentum({0.0f, 0.0f, 1.0f});
  hit.setEDep(eDep);
  hit.setTime(time);
  hit.setPathLength(0.05);
  hit.setParticle(particle);
  return hit;
}

SiliconTrackerDigiConfig configWithThreshold(double threshold) {
  SiliconTrackerDigiConfig cfg;
  cfg.threshold      = threshold;
  cfg.timeResolution = 0.0;
  return cfg;
}

} // namespace

TEST_CASE("SiliconTrackerDigi associations exclude subthreshold deposits", "[SiliconTrackerDigi]") {
  SiliconTrackerDigi algo("silicon_truth_threshold");
  algo.applyConfig(configWithThreshold(10.0e-6)); // 10 keV in GeV
  algo.init();

  edm4hep::EventHeaderCollection headers;
  headers.create(1, 0);
  edm4hep::SimTrackerHitCollection simHits;
  edm4hep::MCParticleCollection particles;

  const std::uint64_t cellID = 42;
  auto accepted = addHit(simHits, particles, cellID, 12.0e-6, 1.0);
  auto rejected = addHit(simHits, particles, cellID, 9.0e-6, 2.0);

  edm4eic::RawTrackerHitCollection rawHits;
  edm4eic::MCRecoTrackerHitLinkCollection links;
  edm4eic::MCRecoTrackerHitAssociationCollection associations;
  algo.process({&headers, &simHits}, {&rawHits, &links, &associations});

  REQUIRE(rawHits.size() == 1);
  CHECK(rawHits[0].getCharge() == 12);
  REQUIRE(associations.size() == 1);
  CHECK(associations[0].getSimHit().id() == accepted.id());
  CHECK(associations[0].getSimHit().id() != rejected.id());
  CHECK(associations[0].getWeight() == Approx(1.0));
}

TEST_CASE("SiliconTrackerDigi association weights follow digitized charge fractions",
          "[SiliconTrackerDigi]") {
  SiliconTrackerDigi algo("silicon_truth_weights");
  algo.applyConfig(configWithThreshold(10.0e-6));
  algo.init();

  edm4hep::EventHeaderCollection headers;
  headers.create(2, 0);
  edm4hep::SimTrackerHitCollection simHits;
  edm4hep::MCParticleCollection particles;

  const std::uint64_t cellID = 43;
  auto first  = addHit(simHits, particles, cellID, 12.0e-6, 2.0);
  auto second = addHit(simHits, particles, cellID, 18.0e-6, 1.0);

  edm4eic::RawTrackerHitCollection rawHits;
  edm4eic::MCRecoTrackerHitLinkCollection links;
  edm4eic::MCRecoTrackerHitAssociationCollection associations;
  algo.process({&headers, &simHits}, {&rawHits, &links, &associations});

  REQUIRE(rawHits.size() == 1);
  CHECK(rawHits[0].getCharge() == 30);
  REQUIRE(associations.size() == 2);
  REQUIRE(links.size() == 2);

  double firstWeight = -1.0;
  double secondWeight = -1.0;
  for (const auto& assoc : associations) {
    if (assoc.getSimHit().id() == first.id()) {
      firstWeight = assoc.getWeight();
    } else if (assoc.getSimHit().id() == second.id()) {
      secondWeight = assoc.getWeight();
    }
  }
  CHECK(firstWeight == Approx(0.4));
  CHECK(secondWeight == Approx(0.6));
  CHECK(firstWeight + secondWeight == Approx(1.0));
}
