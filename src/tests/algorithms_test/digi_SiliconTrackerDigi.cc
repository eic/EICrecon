// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ePIC Collaboration

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/MCRecoTrackerHitLinkCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "algorithms/digi/SiliconTrackerDigi.h"
#include "algorithms/digi/SiliconTrackerDigiConfig.h"

using eicrecon::SiliconTrackerDigi;
using eicrecon::SiliconTrackerDigiConfig;

namespace {

// SiliconTrackerDigi does not consult the geometry, so any cellID will do.
constexpr std::uint64_t kCellID      = 0x123456;
constexpr std::uint64_t kOtherCellID = 0x654321;

void createSimHit(edm4hep::SimTrackerHitCollection& sim_hits,
                  edm4hep::MCParticleCollection& mc_particles, std::uint64_t cellID, double eDep,
                  double time) {
  auto particle = mc_particles.create();
  particle.setPDG(11);
  particle.setGeneratorStatus(1);

  auto hit = sim_hits.create();
  hit.setCellID(cellID);
  hit.setEDep(static_cast<float>(eDep));
  hit.setTime(static_cast<float>(time));
  hit.setParticle(particle);
}

struct DigiOutput {
  edm4eic::RawTrackerHitCollection raw_hits;
  edm4eic::MCRecoTrackerHitLinkCollection links;
  edm4eic::MCRecoTrackerHitAssociationCollection associations;
};

void run(SiliconTrackerDigi& algo, const edm4hep::EventHeaderCollection& headers,
         const edm4hep::SimTrackerHitCollection& sim_hits, DigiOutput& out) {
  algo.process({&headers, &sim_hits}, {&out.raw_hits, &out.links, &out.associations});
}

} // namespace

TEST_CASE("SiliconTrackerDigi: timestamp is the rounded time in ps without smearing",
          "[SiliconTrackerDigi]") {
  SiliconTrackerDigi algo("test_digi_rounding");
  SiliconTrackerDigiConfig cfg;
  cfg.threshold      = 0.0;
  cfg.timeResolution = 0.0;
  algo.applyConfig(cfg);
  algo.init();

  edm4hep::EventHeaderCollection headers;
  headers.create(1, 0);

  // Sim-hit times in ns and the expected timestamps in ps. The half-ps case and the
  // negative time distinguish rounding from truncation towards zero.
  const std::vector<std::pair<double, std::int32_t>> cases = {
      {100.0, 100000},
      {1.0005, 1001},
      {-2.0007, -2001},
  };

  for (const auto& [time, expected] : cases) {
    edm4hep::SimTrackerHitCollection sim_hits;
    edm4hep::MCParticleCollection mc_particles;
    createSimHit(sim_hits, mc_particles, kCellID, 1.0e-6, time);

    DigiOutput out;
    run(algo, headers, sim_hits, out);

    REQUIRE(out.raw_hits.size() == 1);
    CHECK(out.raw_hits[0].getCellID() == kCellID);
    CHECK(out.raw_hits[0].getTimeStamp() == expected);
  }
}

TEST_CASE("SiliconTrackerDigi: time smearing follows the configured resolution",
          "[SiliconTrackerDigi]") {
  SiliconTrackerDigi algo("test_digi_smearing");
  SiliconTrackerDigiConfig cfg;
  cfg.threshold      = 0.0;
  cfg.timeResolution = 1.0 * edm4eic::unit::ns;
  algo.applyConfig(cfg);
  algo.init();

  const double simTime        = 50.0; // ns
  const std::size_t numEvents = 2000;

  double sum   = 0.0;
  double sumSq = 0.0;
  for (std::size_t ievent = 0; ievent < numEvents; ++ievent) {
    edm4hep::EventHeaderCollection headers;
    headers.create(static_cast<int>(ievent), 0);
    edm4hep::SimTrackerHitCollection sim_hits;
    edm4hep::MCParticleCollection mc_particles;
    createSimHit(sim_hits, mc_particles, kCellID, 1.0e-6, simTime);

    DigiOutput out;
    run(algo, headers, sim_hits, out);
    REQUIRE(out.raw_hits.size() == 1);

    const double residual = out.raw_hits[0].getTimeStamp() * edm4eic::unit::ps - simTime;
    sum += residual;
    sumSq += residual * residual;
  }

  const double mean  = sum / numEvents;
  const double sigma = std::sqrt(sumSq / numEvents - mean * mean);
  // Statistical precision with 2000 samples is ~2% on sigma and ~0.02 ns on the mean
  CHECK(std::abs(mean) < 0.1);
  CHECK(sigma == Catch::Approx(1.0).epsilon(0.1));
}

TEST_CASE("SiliconTrackerDigi: smearing is reproducible for the same event",
          "[SiliconTrackerDigi]") {
  SiliconTrackerDigi algo("test_digi_reproducible");
  SiliconTrackerDigiConfig cfg;
  cfg.threshold      = 0.0;
  cfg.timeResolution = 10.0 * edm4eic::unit::ns;
  algo.applyConfig(cfg);
  algo.init();

  edm4hep::EventHeaderCollection headers;
  headers.create(42, 7);
  edm4hep::SimTrackerHitCollection sim_hits;
  edm4hep::MCParticleCollection mc_particles;
  createSimHit(sim_hits, mc_particles, kCellID, 1.0e-6, 10.0);

  DigiOutput first;
  run(algo, headers, sim_hits, first);
  DigiOutput second;
  run(algo, headers, sim_hits, second);

  REQUIRE(first.raw_hits.size() == 1);
  REQUIRE(second.raw_hits.size() == 1);
  CHECK(first.raw_hits[0].getTimeStamp() == second.raw_hits[0].getTimeStamp());
}

TEST_CASE("SiliconTrackerDigi: hits in the same cell keep the earliest time and sum the charge",
          "[SiliconTrackerDigi]") {
  SiliconTrackerDigi algo("test_digi_merge");
  SiliconTrackerDigiConfig cfg;
  cfg.threshold      = 0.0;
  cfg.timeResolution = 0.0;
  algo.applyConfig(cfg);
  algo.init();

  edm4hep::EventHeaderCollection headers;
  headers.create(1, 0);
  edm4hep::SimTrackerHitCollection sim_hits;
  edm4hep::MCParticleCollection mc_particles;
  createSimHit(sim_hits, mc_particles, kCellID, 1.0e-6, 20.0);
  createSimHit(sim_hits, mc_particles, kCellID, 2.0e-6, 15.0);
  createSimHit(sim_hits, mc_particles, kOtherCellID, 3.0e-6, 30.0);

  DigiOutput out;
  run(algo, headers, sim_hits, out);

  REQUIRE(out.raw_hits.size() == 2);
  for (const auto& raw_hit : out.raw_hits) {
    if (raw_hit.getCellID() == kCellID) {
      CHECK(raw_hit.getTimeStamp() == 15000);
      CHECK(raw_hit.getCharge() == 3); // (1 + 2) GeV * 1e6
    } else {
      CHECK(raw_hit.getCellID() == kOtherCellID);
      CHECK(raw_hit.getTimeStamp() == 30000);
      CHECK(raw_hit.getCharge() == 3);
    }
  }
  // Every sim hit is linked to the raw hit of its cell
  CHECK(out.links.size() == 3);
  CHECK(out.associations.size() == 3);
}

TEST_CASE("SiliconTrackerDigi: hits below threshold are dropped", "[SiliconTrackerDigi]") {
  SiliconTrackerDigi algo("test_digi_threshold");
  SiliconTrackerDigiConfig cfg;
  cfg.threshold      = 1.0e-6; // 1 keV in dd4hep units (GeV)
  cfg.timeResolution = 0.0;
  algo.applyConfig(cfg);
  algo.init();

  edm4hep::EventHeaderCollection headers;
  headers.create(1, 0);
  edm4hep::SimTrackerHitCollection sim_hits;
  edm4hep::MCParticleCollection mc_particles;
  createSimHit(sim_hits, mc_particles, kCellID, 0.5e-6, 10.0);
  createSimHit(sim_hits, mc_particles, kOtherCellID, 2.0e-6, 10.0);

  DigiOutput out;
  run(algo, headers, sim_hits, out);

  REQUIRE(out.raw_hits.size() == 1);
  CHECK(out.raw_hits[0].getCellID() == kOtherCellID);
}
