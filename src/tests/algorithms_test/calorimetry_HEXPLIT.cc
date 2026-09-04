// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 - 2026, Sebouh Paul, ePIC Collaboration

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <Evaluator/DD4hepUnits.h>
#include <algorithms/geo.h>
#include <catch2/catch_test_macros.hpp>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/MCRecoCalorimeterHitLinkCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/MutableCaloHitContribution.h>
#include <edm4hep/RawCalorimeterHitCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/Vector3f.h>
#include <podio/detail/Link.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <gsl/pointers>
#include <memory>
#include <numbers>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "algorithms/calorimetry/HEXPLIT.h"
#include "algorithms/calorimetry/HEXPLITConfig.h"

using eicrecon::HEXPLIT;
using eicrecon::HEXPLITConfig;

static constexpr double kSideLength   = 31.3 * dd4hep::mm;
static constexpr double kLayerSpacing = 25.1 * dd4hep::mm;
static constexpr double kThickness    = 3 * dd4hep::mm;

// Five hits in consecutive layers that all overlap in a single rhombus.
static const std::array<double, 5> kX = {0, 0.75 * kSideLength, 0, 0.75 * kSideLength, 0};
static const std::array<double, 5> kY = {
    std::numbers::sqrt3 / 2 * kSideLength, -0.25 * std::numbers::sqrt3* kSideLength, 0,
    0.25 * std::numbers::sqrt3* kSideLength, std::numbers::sqrt3 / 2 * kSideLength};

static edm4hep::Vector3f cell_dimension() {
  return {(float)(2 * kSideLength), (float)(std::numbers::sqrt3 * kSideLength), (float)kThickness};
}

static uint64_t mock_cell_id() {
  return algorithms::GeoSvc::instance()
      .detector()
      ->readout("MockCalorimeterHits")
      .idSpec()
      .encode({{"system", 255}, {"x", 0}, {"y", 0}});
}

// Configure and initialise the algorithm.
static std::unique_ptr<HEXPLIT> make_algo(double max_time_to_truth_t0_ns) {
  auto algo = std::make_unique<HEXPLIT>("HEXPLIT");
  spdlog::default_logger()->clone("HEXPLIT")->set_level(spdlog::level::warn);
  HEXPLITConfig cfg;
  cfg.MIP                  = 472. * dd4hep::keV;
  cfg.max_time_to_truth_t0 = max_time_to_truth_t0_ns * dd4hep::ns;
  algo->applyConfig(cfg);
  algo->init();
  return algo;
}

// Owns all PODIO collections that make up a single hit's truth chain:
//   primary (status=1, time=t0_ns)
//   └─ Geant4 secondary (status=0)
//      └─ SimCalorimeterHit contribution
//         ← MCRecoCalorimeterHitLink →
//         RawCalorimeterHit
//         └─ CalorimeterHit (hit_time_ns)
struct TruthChain {
  edm4hep::MCParticleCollection mcparticles;
  edm4hep::SimCalorimeterHitCollection simhits;
  edm4hep::RawCalorimeterHitCollection rawhits;
  edm4eic::MCRecoCalorimeterHitLinkCollection links;
  edm4eic::CalorimeterHitCollection rechits;
};

static TruthChain make_truth_chain(uint64_t cellID, float t0_ns, float hit_time_ns, float energy,
                                   edm4hep::Vector3f pos, edm4hep::Vector3f dim, int32_t layer) {
  TruthChain c;

  auto primary = c.mcparticles.create();
  primary.setGeneratorStatus(1);
  primary.setTime(t0_ns);

  auto secondary = c.mcparticles.create();
  secondary.setGeneratorStatus(0);
  secondary.addToParents(primary);

  auto simhit = c.simhits.create();
  simhit.setCellID(cellID);
  edm4hep::MutableCaloHitContribution contrib;
  contrib.setParticle(secondary);
  contrib.setTime(hit_time_ns);
  contrib.setEnergy(energy);
  simhit.addToContributions(contrib);

  auto rawhit = c.rawhits.create();

  auto link = c.links.create();
  link.setFrom(rawhit);
  link.setTo(simhit);

  auto rechit = c.rechits.create(cellID, energy, 0.f, hit_time_ns, 0.f, pos, dim, 0, layer, pos);
  rechit.setRawHit(rawhit);

  return c;
}

// Build the standard 5-hit geometry with truth links at the given t0 / hit time.
// Returns (hits, links, chains) — chains must remain alive while hits/links are used,
// since they own the rawhit/simhit collections that hits and links reference.
static std::tuple<edm4eic::CalorimeterHitCollection, edm4eic::MCRecoCalorimeterHitLinkCollection,
                  std::vector<TruthChain>>
make_linked_hits(float t0_ns, float hit_time_ns) {
  const uint64_t cellID       = mock_cell_id();
  const edm4hep::Vector3f dim = cell_dimension();
  constexpr float E           = 50 * dd4hep::MeV;

  std::vector<TruthChain> chains;
  edm4eic::CalorimeterHitCollection hits;
  edm4eic::MCRecoCalorimeterHitLinkCollection links;

  for (std::size_t i = 0; i < 5; ++i) {
    edm4hep::Vector3f pos(kX[i], kY[i], i * kLayerSpacing);
    auto& c =
        chains.emplace_back(make_truth_chain(cellID, t0_ns, hit_time_ns, E, pos, dim, (int32_t)i));

    auto rh = hits.create(cellID, E, 0.f, hit_time_ns, 0.f, pos, dim, 0, (int32_t)i, pos);
    rh.setRawHit(c.rawhits[0]);

    auto lk = links.create();
    lk.setFrom(c.rawhits[0]);
    lk.setTo(c.simhits[0]);
  }
  return {std::move(hits), std::move(links), std::move(chains)};
}

TEST_CASE("the subcell-splitting algorithm runs", "[HEXPLIT]") {
  auto algo = make_algo(/*max_time_to_truth_t0_ns=*/1000.);

  const uint64_t cellID       = mock_cell_id();
  const edm4hep::Vector3f dim = cell_dimension();

  edm4eic::CalorimeterHitCollection hits_coll;
  std::array<double, 5> E = {50 * dd4hep::MeV, 50 * dd4hep::MeV, 50 * dd4hep::MeV, 50 * dd4hep::MeV,
                             50 * dd4hep::MeV};
  for (std::size_t i = 0; i < 5; i++) {
    edm4hep::Vector3f pos(kX[i], kY[i], i * kLayerSpacing);
    hits_coll.create(cellID, E[i], 0., 0., 0., pos, dim, 0, (int32_t)i, pos);
  }

  auto subcellhits_coll = std::make_unique<edm4eic::CalorimeterHitCollection>();
  algo->process({&hits_coll, nullptr}, {subcellhits_coll.get()});

  // 12 subcells per cell × 5 cells
  REQUIRE(subcellhits_coll->size() == 60);

  // energy is conserved per cell
  double tol = 0.001, Esum = 0;
  int idx = 0;
  for (auto subcell : *subcellhits_coll) {
    Esum += subcell.getEnergy();
    if (++idx % 12 == 0) {
      REQUIRE(std::abs(Esum - E[idx / 12 - 1]) / E[idx / 12 - 1] < tol);
      Esum = 0;
    }
  }
  // almost all energy of the middle-layer hit ends up in the overlapping subcell
  REQUIRE((*subcellhits_coll)[35].getEnergy() / E[2] > 0.95);
}

TEST_CASE("HEXPLIT timing cut: no link collection skips the cut", "[HEXPLIT]") {
  auto algo = make_algo(500.);
  // Hits 1000 ns late — well outside the 500 ns window — but cut is skipped
  // when no link collection is provided.
  auto [hits, links, chains] = make_linked_hits(0.f, 1000.f);
  auto out                   = std::make_unique<edm4eic::CalorimeterHitCollection>();
  algo->process({&hits, nullptr}, {out.get()});
  REQUIRE(out->size() == 60);
}

TEST_CASE("HEXPLIT timing cut: hits within window pass", "[HEXPLIT]") {
  auto algo = make_algo(500.);
  // dt = 100 ns - 0 ns = 100 ns < 500 ns → all pass
  auto [hits, links, chains] = make_linked_hits(/*t0=*/0.f, /*hit_time=*/100.f);
  auto out                   = std::make_unique<edm4eic::CalorimeterHitCollection>();
  algo->process({&hits, &links}, {out.get()});
  REQUIRE(out->size() == 60);
}

TEST_CASE("HEXPLIT timing cut: hits outside window are rejected", "[HEXPLIT]") {
  auto algo = make_algo(500.);
  // dt = 1000 ns - 0 ns = 1000 ns > 500 ns → all rejected
  auto [hits, links, chains] = make_linked_hits(/*t0=*/0.f, /*hit_time=*/1000.f);
  auto out                   = std::make_unique<edm4eic::CalorimeterHitCollection>();
  algo->process({&hits, &links}, {out.get()});
  REQUIRE(out->size() == 0);
}

TEST_CASE("HEXPLIT timing cut: 1 us vertex offset correctly subtracted", "[HEXPLIT]") {
  auto algo = make_algo(500.);
  // dt = 1100 ns - 1000 ns = 100 ns < 500 ns → all pass
  auto [hits, links, chains] = make_linked_hits(/*t0=*/1000.f, /*hit_time=*/1100.f);
  auto out                   = std::make_unique<edm4eic::CalorimeterHitCollection>();
  algo->process({&hits, &links}, {out.get()});
  REQUIRE(out->size() == 60);
}

TEST_CASE("HEXPLIT timing cut: 1 us offset with late hits rejected", "[HEXPLIT]") {
  auto algo = make_algo(500.);
  // dt = 2000 ns - 1000 ns = 1000 ns > 500 ns → all rejected
  auto [hits, links, chains] = make_linked_hits(/*t0=*/1000.f, /*hit_time=*/2000.f);
  auto out                   = std::make_unique<edm4eic::CalorimeterHitCollection>();
  algo->process({&hits, &links}, {out.get()});
  REQUIRE(out->size() == 0);
}
