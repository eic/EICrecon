// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Chun Yuen Tsang, Minho Kim

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <algorithms/geo.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/MCRecoCalorimeterHitAssociationCollection.h>
#include <edm4eic/MCRecoCalorimeterHitLinkCollection.h>
#include <edm4hep/CaloHitContributionCollection.h>
#include <edm4hep/MCParticle.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/RawCalorimeterHitCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/Vector3f.h>
#include <podio/detail/LinkCollectionIterator.h>
#include <stdint.h>
#include <algorithm>
#include <cmath>
#include <deque>
#include <gsl/pointers>
#include <initializer_list>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/calorimetry/CalorimeterCALOROCRecoCombiner.h"
#include "algorithms/calorimetry/CalorimeterCALOROCRecoCombinerConfig.h"

namespace {

// Helper: a single-sided hit as produced by CalorimeterCALOROCReco
edm4eic::MutableCalorimeterHit make_side_hit(edm4eic::CalorimeterHitCollection& recColl,
                                             edm4hep::RawCalorimeterHitCollection& rawColl,
                                             uint64_t cellID, float energy, float time,
                                             int32_t amplitude) {
  auto raw = rawColl.create();
  raw.setCellID(cellID);
  raw.setAmplitude(amplitude);
  raw.setTimeStamp(static_cast<int32_t>(time));

  auto hit = recColl.create();
  hit.setCellID(cellID);
  hit.setEnergy(energy);
  hit.setTime(time);
  hit.setPosition({1.f, 2.f, 0.f});
  hit.setDimension({3.f, 4.f, 5.f});
  hit.setSector(6);
  hit.setLayer(7);
  hit.setRawHit(raw);
  return hit;
}

edm4hep::MutableSimCalorimeterHit
make_npe_hit(edm4hep::SimCalorimeterHitCollection& hitColl,
             edm4hep::CaloHitContributionCollection& contribColl, uint64_t cellID,
             edm4hep::Vector3f position, std::initializer_list<float> contrib_energies,
             std::initializer_list<edm4hep::MCParticle> particles = {}) {
  REQUIRE((particles.size() == 0 || particles.size() == contrib_energies.size()));

  float total = 0.f;
  for (auto e : contrib_energies)
    total += e;
  auto hit = hitColl.create();
  hit.setCellID(cellID);
  hit.setEnergy(total);
  hit.setPosition(position);
  auto particle = particles.begin();
  for (auto e : contrib_energies) {
    auto c = contribColl.create();
    c.setEnergy(e);
    c.setTime(0.f);
    c.setStepPosition({0.f, 0.f, 0.f});
    if (particle != particles.end())
      c.setParticle(*particle++);
    hit.addToContributions(c);
  }
  return hit;
}

eicrecon::CalorimeterCALOROCRecoCombinerConfig make_cfg() {
  eicrecon::CalorimeterCALOROCRecoCombinerConfig cfg;
  cfg.readout        = "MockCALOROCHits";
  cfg.localDetFields = {"system"};
  cfg.useNpeHitPos   = false;
  cfg.attenuationReferencePositionNamePos =
      "MockCALOROC_RefPosP"; // "1000.0" in dd4hep units = 10000 mm
  cfg.attenuationReferencePositionNameNeg = "MockCALOROC_RefPosN"; // -10000 mm
  // Identity-like attenuation: factor ≈ 1 for all reasonable z
  cfg.attenuationParameters = {1.0, 1.0e9, 1.0e9};
  // z = dt * 1.0 + 0.0
  cfg.lightSpeedParameters = {1.0, 0.0};
  return cfg;
}

uint64_t make_cell_id() {
  auto detector = algorithms::GeoSvc::instance().detector();
  auto id_desc  = detector->readout("MockCALOROCHits").idSpec();
  return id_desc.encode({{"system", 4}, {"layer", 0}, {"sector", 0}, {"x", 0}, {"y", 0}});
}

// Everything a test needs for the inputs/outputs
struct Event {
  edm4eic::CalorimeterHitCollection recP, recN;
  edm4hep::RawCalorimeterHitCollection rawP, rawN;
  edm4hep::SimCalorimeterHitCollection npeP, npeN;
  edm4hep::CaloHitContributionCollection contribs;

  std::unique_ptr<edm4eic::CalorimeterHitCollection> recohits =
      std::make_unique<edm4eic::CalorimeterHitCollection>();
  std::unique_ptr<edm4hep::RawCalorimeterHitCollection> rawhits =
      std::make_unique<edm4hep::RawCalorimeterHitCollection>();
  edm4eic::MCRecoCalorimeterHitLinkCollection links;
  edm4eic::MCRecoCalorimeterHitAssociationCollection assocs;

  void run(eicrecon::CalorimeterCALOROCRecoCombiner& algo) {
    algo.process({&recP, &recN, &npeP, &npeN}, {recohits.get(), rawhits.get(), &links, &assocs});
  }
};

} // namespace

TEST_CASE("CalorimeterCALOROCRecoCombiner: z-position from timing difference",
          "[CalorimeterCALOROCRecoCombiner][ZPosition]") {

  eicrecon::CalorimeterCALOROCRecoCombiner algo("CALOROCCombinerZPos");
  algo.applyConfig(make_cfg());
  algo.init();

  const uint64_t cellID = make_cell_id();

  // tP = -12.5, tN = 12.5 -> dt = 25 -> z = 25
  Event ev;
  make_side_hit(ev.recP, ev.rawP, cellID, 10.f, -12.5f, 100);
  make_side_hit(ev.recN, ev.rawN, cellID, 10.f, 12.5f, 200);
  make_npe_hit(ev.npeP, ev.contribs, cellID, {0.f, 0.f, 0.f}, {60.f});
  make_npe_hit(ev.npeN, ev.contribs, cellID, {0.f, 0.f, 0.f}, {40.f});
  ev.run(algo);

  REQUIRE(ev.recohits->size() == 1);
  const auto hit = (*ev.recohits)[0];
  REQUIRE_THAT(hit.getPosition().z, Catch::Matchers::WithinAbs(25.0, 1e-4));
  // x, y, dimension, sector and layer are inherited from the single-sided hit
  REQUIRE_THAT(hit.getPosition().x, Catch::Matchers::WithinAbs(1.0, 1e-4));
  REQUIRE_THAT(hit.getPosition().y, Catch::Matchers::WithinAbs(2.0, 1e-4));
  REQUIRE_THAT(hit.getDimension().x, Catch::Matchers::WithinAbs(3.0, 1e-4));
  REQUIRE(hit.getSector() == 6);
  REQUIRE(hit.getLayer() == 7);
}

TEST_CASE("CalorimeterCALOROCRecoCombiner: z-position is bounded by the SiPM positions",
          "[CalorimeterCALOROCRecoCombiner][ZPosition]") {

  eicrecon::CalorimeterCALOROCRecoCombiner algo("CALOROCCombinerZBound");
  algo.applyConfig(make_cfg());
  algo.init();

  const uint64_t cellID = make_cell_id();

  // dt = 50000 ns -> z = 50000 mm, outside of [-10000, 10000]
  Event ev;
  make_side_hit(ev.recP, ev.rawP, cellID, 10.f, 0.f, 100);
  make_side_hit(ev.recN, ev.rawN, cellID, 10.f, 50000.f, 200);
  make_npe_hit(ev.npeP, ev.contribs, cellID, {0.f, 0.f, 0.f}, {60.f});
  make_npe_hit(ev.npeN, ev.contribs, cellID, {0.f, 0.f, 0.f}, {40.f});
  ev.run(algo);

  REQUIRE(ev.recohits->size() == 1);
  REQUIRE_THAT((*ev.recohits)[0].getPosition().z, Catch::Matchers::WithinAbs(10000.0, 1e-4));
}

TEST_CASE("CalorimeterCALOROCRecoCombiner: z-position read from NpeHit position",
          "[CalorimeterCALOROCRecoCombiner][ZPosition]") {

  auto cfg         = make_cfg();
  cfg.useNpeHitPos = true;
  eicrecon::CalorimeterCALOROCRecoCombiner algo("CALOROCCombinerNpePos");
  algo.applyConfig(cfg);
  algo.init();

  const uint64_t cellID = make_cell_id();

  Event ev;
  make_side_hit(ev.recP, ev.rawP, cellID, 10.f, -12.5f, 100);
  make_side_hit(ev.recN, ev.rawN, cellID, 10.f, 12.5f, 200);
  // NpeHitP has z = 42.0 mm: this is used instead of the timing
  make_npe_hit(ev.npeP, ev.contribs, cellID, {0.f, 0.f, 42.f}, {60.f});
  make_npe_hit(ev.npeN, ev.contribs, cellID, {0.f, 0.f, 0.f}, {40.f});
  ev.run(algo);

  REQUIRE(ev.recohits->size() == 1);
  REQUIRE_THAT((*ev.recohits)[0].getPosition().z, Catch::Matchers::WithinAbs(42.0, 1e-4));
}

TEST_CASE("CalorimeterCALOROCRecoCombiner: attenuation correction and geometric mean",
          "[CalorimeterCALOROCRecoCombiner][Energy]") {

  auto cfg = make_cfg();
  // single exponential, attenuation length 1000 mm: factor = exp(-L / 1000)
  cfg.attenuationParameters = {1.0, 1000.0, 1000.0};
  eicrecon::CalorimeterCALOROCRecoCombiner algo("CALOROCCombinerEnergy");
  algo.applyConfig(cfg);
  algo.init();

  const uint64_t cellID = make_cell_id();

  // z = 25: L_P = 10000 - 25 = 9975, L_N = 25 + 10000 = 10025
  // E = sqrt(10 e^{9.975} * 10 e^{10.025}) = 10 e^{10}
  Event ev;
  make_side_hit(ev.recP, ev.rawP, cellID, 10.f, -12.5f, 100);
  make_side_hit(ev.recN, ev.rawN, cellID, 10.f, 12.5f, 200);
  make_npe_hit(ev.npeP, ev.contribs, cellID, {0.f, 0.f, 0.f}, {60.f});
  make_npe_hit(ev.npeN, ev.contribs, cellID, {0.f, 0.f, 0.f}, {40.f});
  ev.run(algo);

  REQUIRE(ev.recohits->size() == 1);
  REQUIRE_THAT((*ev.recohits)[0].getEnergy(),
               Catch::Matchers::WithinRel(static_cast<float>(10.0 * std::exp(10.0)), 1e-5f));
}

TEST_CASE("CalorimeterCALOROCRecoCombiner: time is the average of both sides",
          "[CalorimeterCALOROCRecoCombiner][Timing]") {

  eicrecon::CalorimeterCALOROCRecoCombiner algo("CALOROCCombinerTime");
  algo.applyConfig(make_cfg());
  algo.init();

  const uint64_t cellID = make_cell_id();

  Event ev;
  make_side_hit(ev.recP, ev.rawP, cellID, 10.f, -10.f, 100);
  make_side_hit(ev.recN, ev.rawN, cellID, 10.f, 30.f, 200);
  make_npe_hit(ev.npeP, ev.contribs, cellID, {0.f, 0.f, 0.f}, {60.f});
  make_npe_hit(ev.npeN, ev.contribs, cellID, {0.f, 0.f, 0.f}, {40.f});
  ev.run(algo);

  REQUIRE(ev.recohits->size() == 1);
  REQUIRE(ev.rawhits->size() == 1);
  REQUIRE_THAT((*ev.recohits)[0].getTime(), Catch::Matchers::WithinAbs(10.0, 1e-4));
  // the combined raw hit keeps the P-side amplitude and the combined time
  REQUIRE((*ev.rawhits)[0].getAmplitude() == 100);
  REQUIRE((*ev.rawhits)[0].getTimeStamp() == 10);
  REQUIRE((*ev.recohits)[0].getRawHit() == (*ev.rawhits)[0]);
}

TEST_CASE("CalorimeterCALOROCRecoCombiner: MC truth link weights are energy-normalized",
          "[CalorimeterCALOROCRecoCombiner][MCLinks]") {

  eicrecon::CalorimeterCALOROCRecoCombiner algo("CALOROCCombinerLinks");
  algo.applyConfig(make_cfg());
  algo.init();

  const uint64_t cellID = make_cell_id();

  Event ev;
  make_side_hit(ev.recP, ev.rawP, cellID, 10.f, 0.f, 100);
  make_side_hit(ev.recN, ev.rawN, cellID, 10.f, 0.f, 200);

  // NpeHitP has 2 contributions: 30 and 70, NpeHitN has 1 contribution: 50
  // Total edep = 30 + 70 + 50 = 150
  edm4hep::MCParticleCollection particles;
  auto particleP1 = particles.create();
  auto particleP2 = particles.create();
  auto particleN1 = particles.create();
  make_npe_hit(ev.npeP, ev.contribs, cellID, {0.f, 0.f, 0.f}, {30.f, 70.f},
               {particleP1, particleP2});
  make_npe_hit(ev.npeN, ev.contribs, cellID, {0.f, 0.f, 0.f}, {50.f}, {particleN1});
  ev.run(algo);

  std::vector<double> weights;
  for (const auto& lnk : ev.links)
    weights.push_back(lnk.getWeight());
  std::sort(weights.begin(), weights.end());
  double total_weight = 0.;
  for (auto w : weights)
    total_weight += w;

  REQUIRE(ev.links.size() == 3);
  REQUIRE(ev.assocs.size() == 3);
  REQUIRE_THAT(total_weight, Catch::Matchers::WithinAbs(1.0, 1e-5));
  REQUIRE_THAT(weights[0], Catch::Matchers::WithinAbs(30.0 / 150.0, 1e-5));
  REQUIRE_THAT(weights[1], Catch::Matchers::WithinAbs(50.0 / 150.0, 1e-5));
  REQUIRE_THAT(weights[2], Catch::Matchers::WithinAbs(70.0 / 150.0, 1e-5));
}

TEST_CASE("CalorimeterCALOROCRecoCombiner: hit without the opposite side is dropped",
          "[CalorimeterCALOROCRecoCombiner][Matching]") {

  eicrecon::CalorimeterCALOROCRecoCombiner algo("CALOROCCombinerMatching");
  algo.applyConfig(make_cfg());
  algo.init();

  const uint64_t cellID = make_cell_id();

  Event ev; // no N-side hit
  make_side_hit(ev.recP, ev.rawP, cellID, 10.f, 0.f, 100);
  make_npe_hit(ev.npeP, ev.contribs, cellID, {0.f, 0.f, 0.f}, {60.f});
  make_npe_hit(ev.npeN, ev.contribs, cellID, {0.f, 0.f, 0.f}, {40.f});
  ev.run(algo);

  REQUIRE(ev.recohits->empty());
  REQUIRE(ev.rawhits->empty());
}
