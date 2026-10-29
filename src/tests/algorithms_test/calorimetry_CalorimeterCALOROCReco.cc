// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Chun Yuen Tsang, Minho Kim

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <algorithms/geo.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <edm4eic/CALOROC1BSample.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/RawCALOROCHitCollection.h>
#include <edm4hep/CaloHitContributionCollection.h>
#include <edm4hep/MCParticle.h>
#include <edm4hep/RawCalorimeterHitCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/Vector3f.h>
#include <podio/detail/LinkCollectionIterator.h>
#include <stdint.h>
#include <algorithm>
#include <deque>
#include <filesystem>
#include <fstream>
#include <gsl/pointers>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/calorimetry/CalorimeterCALOROCReco.h"
#include "algorithms/calorimetry/CalorimeterCALOROCRecoConfig.h"

// Helper: build a RawCALOROCHit with one B-sample
// timeStamp and samplePhase control the TOA formula; highGainADC/lowGainADC control amplitude
static edm4eic::MutableRawCALOROCHit make_raw_hit(edm4eic::RawCALOROCHitCollection& coll,
                                                  uint64_t cellID, int32_t timeStamp,
                                                  int32_t samplePhase, uint16_t highGainADC,
                                                  uint16_t lowGainADC, uint16_t timeOfArrival) {
  auto hit = coll.create();
  hit.setCellID(cellID);
  hit.setTimeStamp(timeStamp);
  hit.setSamplePhase(samplePhase);
  edm4eic::CALOROC1BSample sample;
  sample.highGainADC   = highGainADC;
  sample.lowGainADC    = lowGainADC;
  sample.timeOfArrival = timeOfArrival;
  hit.addToBSamples(sample);
  return hit;
}

// Helper: build an NpeHit (SimCalorimeterHit) with given contributions
static edm4hep::MutableSimCalorimeterHit
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

// Write a minimal LUT file with one entry: layer=0 sector=0 -> factor
// Returns the file path as a string
static std::string write_lut_file(const std::string& path, double factor = 1.0) {
  std::ofstream f(path);
  f << "0 0 " << factor << "\n";
  return path;
}

// Convenience: build the standard config used in most tests
static eicrecon::CalorimeterCALOROCRecoConfig make_cfg(const std::string& lut_path) {
  eicrecon::CalorimeterCALOROCRecoConfig cfg;
  cfg.readout              = "MockCALOROCHits";
  cfg.layerField           = "layer";
  cfg.sectorField          = "sector";
  cfg.localDetFields       = {"system"};
  cfg.edep_to_npe_filename = lut_path;
  cfg.edep_to_npe_fields   = {"layer", "sector"};

  cfg.slope     = 1.0;
  cfg.intercept = 0.0;

  cfg.highGainDR = 1000; // saturation threshold
  cfg.gainRatio  = 1.0;  // high/low gain ratio = 1 for simplicity

  cfg.timeWalkCor                  = false; // disabled by default; enabled per test-case
  cfg.timeWalkCorrectionParameters = {0.0, 1.0, 0.0, 1.0};
  // correction = toa - (1.0 * (ADCsum - 0)^1 + 0) = toa - ADCsum

  cfg.proxy_type = eicrecon::CalorimeterCALOROCRecoConfig::ProxyType::sum;

  return cfg;
}

// ── TOA formula (for expected-value computation in tests) ──────────────────────
// toa = samplePhase*(25/1042) + (timeStamp + idx_toa)*25 - timeOfArrival*(25/1024)
static double expected_toa(int samplePhase, int timeStamp, int idx_toa, uint16_t toa_val) {
  return samplePhase * (25.0 / 1042.0) + (timeStamp + idx_toa) * 25.0 - toa_val * (25.0 / 1024.0);
}

static uint64_t make_cell_id() {
  auto detector = algorithms::GeoSvc::instance().detector();
  auto id_desc  = detector->readout("MockCALOROCHits").idSpec();
  // layer=0, sector=0 (matches LUT entry)
  return id_desc.encode({{"system", 4}, {"layer", 0}, {"sector", 0}, {"x", 0}, {"y", 0}});
}

// ────────────────────────────────────────────────────────────────────────────────
// Test 1: Amplitude from ADC sum (no saturation)
// ────────────────────────────────────────────────────────────────────────────────
TEST_CASE("CalorimeterCALOROCReco: ADC amplitude in low-signal regime",
          "[CalorimeterCALOROCReco][Amplitude]") {

  const std::string lut_path =
      std::filesystem::temp_directory_path() / "caloroc_test_lut_amplitude.txt";
  write_lut_file(lut_path);

  eicrecon::CalorimeterCALOROCReco algo("CALOROCRecoAmplitude");
  algo.applyConfig(make_cfg(lut_path));
  algo.init();

  const uint64_t cellID = make_cell_id();

  // highGainADC=100, below highGainDR=1000 -> npe = 100/1.0 = 100
  edm4eic::RawCALOROCHitCollection adc_coll;
  make_raw_hit(adc_coll, cellID, /*timeStamp=*/0, /*samplePhase=*/0,
               /*highGainADC=*/100, /*lowGainADC=*/50, /*timeOfArrival=*/512);

  edm4hep::SimCalorimeterHitCollection npe_coll;
  edm4hep::CaloHitContributionCollection contribs_coll;
  make_npe_hit(npe_coll, contribs_coll, cellID, {0.f, 0.f, 0.f}, {60.f});

  auto recohits = std::make_unique<edm4eic::CalorimeterHitCollection>();
  auto rawhits  = std::make_unique<edm4hep::RawCalorimeterHitCollection>();

  algo.process({&npe_coll, &adc_coll}, {recohits.get(), rawhits.get()});

  REQUIRE(rawhits->size() == 1);
  REQUIRE(recohits->size() == 1);
  REQUIRE((*rawhits)[0].getAmplitude() == 100);
  REQUIRE((*recohits)[0].getCellID() == cellID);
  REQUIRE((*recohits)[0].getRawHit() == (*rawhits)[0]);
}

// ────────────────────────────────────────────────────────────────────────────────
// Test 2: Amplitude uses lowGainADC when highGain saturates
// ────────────────────────────────────────────────────────────────────────────────
TEST_CASE("CalorimeterCALOROCReco: ADC amplitude switches to low-gain on saturation",
          "[CalorimeterCALOROCReco][Amplitude]") {

  const std::string lut_path =
      std::filesystem::temp_directory_path() / "caloroc_test_lut_saturation.txt";
  write_lut_file(lut_path);

  eicrecon::CalorimeterCALOROCReco algo("CALOROCRecoSaturation");
  algo.applyConfig(make_cfg(lut_path));
  algo.init();

  const uint64_t cellID = make_cell_id();

  // highGainADC=1000 >= highGainDR=1000 -> use lowGainADC=75 directly
  edm4eic::RawCALOROCHitCollection adc_coll;
  make_raw_hit(adc_coll, cellID, 0, 0, /*highGainADC=*/1000, /*lowGainADC=*/75, /*toa=*/512);

  edm4hep::SimCalorimeterHitCollection npe_coll;
  edm4hep::CaloHitContributionCollection contribs_coll;
  make_npe_hit(npe_coll, contribs_coll, cellID, {0.f, 0.f, 0.f}, {60.f});

  auto recohits = std::make_unique<edm4eic::CalorimeterHitCollection>();
  auto rawhits  = std::make_unique<edm4hep::RawCalorimeterHitCollection>();

  algo.process({&npe_coll, &adc_coll}, {recohits.get(), rawhits.get()});

  REQUIRE(rawhits->size() == 1);
  REQUIRE((*rawhits)[0].getAmplitude() == 75);
}

// ────────────────────────────────────────────────────────────────────────────────
// Test 3: Energy = (npe / layer gain) * slope + intercept
// ────────────────────────────────────────────────────────────────────────────────
TEST_CASE("CalorimeterCALOROCReco: energy from layer gain and linear transformation",
          "[CalorimeterCALOROCReco][Energy]") {

  const std::string lut_path =
      std::filesystem::temp_directory_path() / "caloroc_test_lut_energy.txt";
  write_lut_file(lut_path, 2.0); // 2 npe per unit of edep on this layer

  auto cfg      = make_cfg(lut_path);
  cfg.slope     = 3.0;
  cfg.intercept = 5.0;

  eicrecon::CalorimeterCALOROCReco algo("CALOROCRecoEnergy");
  algo.applyConfig(cfg);
  algo.init();

  const uint64_t cellID = make_cell_id();

  edm4eic::RawCALOROCHitCollection adc_coll;
  make_raw_hit(adc_coll, cellID, 0, 0, 100, 50, 512); // npe = 100

  edm4hep::SimCalorimeterHitCollection npe_coll;
  edm4hep::CaloHitContributionCollection contribs_coll;
  make_npe_hit(npe_coll, contribs_coll, cellID, {0.f, 0.f, 0.f}, {60.f});

  auto recohits = std::make_unique<edm4eic::CalorimeterHitCollection>();
  auto rawhits  = std::make_unique<edm4hep::RawCalorimeterHitCollection>();

  algo.process({&npe_coll, &adc_coll}, {recohits.get(), rawhits.get()});

  // (100 / 2) * 3 + 5 = 155
  REQUIRE(recohits->size() == 1);
  REQUIRE_THAT((*recohits)[0].getEnergy(), Catch::Matchers::WithinAbs(155.0, 1e-4));
}

// ────────────────────────────────────────────────────────────────────────────────
// Test 4: Time of arrival, with and without time walk correction
// ────────────────────────────────────────────────────────────────────────────────
TEST_CASE("CalorimeterCALOROCReco: time walk correction of the time of arrival",
          "[CalorimeterCALOROCReco][TimeWalk]") {

  const std::string lut_path =
      std::filesystem::temp_directory_path() / "caloroc_test_lut_timewalk.txt";
  write_lut_file(lut_path);

  const uint64_t cellID = make_cell_id();

  // ADC sum = 100 (highGainADC=100 < 1000, gainRatio=1)
  edm4eic::RawCALOROCHitCollection adc_coll;
  make_raw_hit(adc_coll, cellID, 1, 0, 100, 50, 512);

  edm4hep::SimCalorimeterHitCollection npe_coll;
  edm4hep::CaloHitContributionCollection contribs_coll;
  make_npe_hit(npe_coll, contribs_coll, cellID, {0.f, 0.f, 0.f}, {60.f});

  const double toa_raw = expected_toa(0, 1, 0, 512); // 12.5

  // Without time walk correction: the time is the raw time of arrival
  {
    auto cfg        = make_cfg(lut_path);
    cfg.timeWalkCor = false;
    eicrecon::CalorimeterCALOROCReco algo("CALOROCRecoNoTWC");
    algo.applyConfig(cfg);
    algo.init();

    auto recohits = std::make_unique<edm4eic::CalorimeterHitCollection>();
    auto rawhits  = std::make_unique<edm4hep::RawCalorimeterHitCollection>();
    algo.process({&npe_coll, &adc_coll}, {recohits.get(), rawhits.get()});

    REQUIRE(recohits->size() == 1);
    REQUIRE_THAT((*recohits)[0].getTime(), Catch::Matchers::WithinAbs(toa_raw, 1e-4));
  }

  // With time walk correction (params = {0, 1, 0, 1}):
  // correction = toa - (p[1] * (ADCsum - p[2])^p[3] + p[0]) = toa - ADCsum = 12.5 - 100
  {
    auto cfg                         = make_cfg(lut_path);
    cfg.timeWalkCor                  = true;
    cfg.timeWalkCorrectionParameters = {0.0, 1.0, 0.0, 1.0};
    eicrecon::CalorimeterCALOROCReco algo("CALOROCRecoWithTWC");
    algo.applyConfig(cfg);
    algo.init();

    auto recohits = std::make_unique<edm4eic::CalorimeterHitCollection>();
    auto rawhits  = std::make_unique<edm4hep::RawCalorimeterHitCollection>();
    algo.process({&npe_coll, &adc_coll}, {recohits.get(), rawhits.get()});

    REQUIRE(recohits->size() == 1);
    REQUIRE_THAT((*recohits)[0].getTime(), Catch::Matchers::WithinAbs(toa_raw - 100.0, 1e-4));
  }
}

// ────────────────────────────────────────────────────────────────────────────────
// Test 5: Hits without a matching ADC hit are dropped
// ────────────────────────────────────────────────────────────────────────────────
TEST_CASE("CalorimeterCALOROCReco: NpeHit without ADC hit is dropped",
          "[CalorimeterCALOROCReco][Matching]") {

  const std::string lut_path =
      std::filesystem::temp_directory_path() / "caloroc_test_lut_matching.txt";
  write_lut_file(lut_path);

  eicrecon::CalorimeterCALOROCReco algo("CALOROCRecoMatching");
  algo.applyConfig(make_cfg(lut_path));
  algo.init();

  const uint64_t cellID = make_cell_id();

  edm4eic::RawCALOROCHitCollection adc_coll; // empty
  edm4hep::SimCalorimeterHitCollection npe_coll;
  edm4hep::CaloHitContributionCollection contribs_coll;
  make_npe_hit(npe_coll, contribs_coll, cellID, {0.f, 0.f, 0.f}, {60.f});

  auto recohits = std::make_unique<edm4eic::CalorimeterHitCollection>();
  auto rawhits  = std::make_unique<edm4hep::RawCalorimeterHitCollection>();

  algo.process({&npe_coll, &adc_coll}, {recohits.get(), rawhits.get()});

  REQUIRE(recohits->empty());
  REQUIRE(rawhits->empty());
}
