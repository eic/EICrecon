// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Chun Yuen Tsang, Minho Kim

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <edm4eic/EDM4eicVersion.h>

#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 7)

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <Evaluator/DD4hepUnits.h>
#include <algorithms/geo.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/MCRecoCalorimeterHitAssociationCollection.h>
#include <edm4eic/MCRecoCalorimeterHitLinkCollection.h>
#include <edm4eic/RawCALOROCHitCollection.h>
#include <edm4eic/CALOROC1BSample.h>
#include <edm4hep/CaloHitContributionCollection.h>
#include <edm4hep/RawCalorimeterHitCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/Vector3f.h>

#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <algorithm>

#include "algorithms/calorimetry/CalorimeterCALOROCCalibration.h"
#include "algorithms/calorimetry/CalorimeterCALOROCCalibrationConfig.h"

// Helper: build a RawCALOROCHit with one B-sample
// timeStamp and samplePhase control the TOA formula; highGainADC/lowGainADC control amplitude
static edm4eic::MutableRawCALOROCHit make_raw_hit(
    edm4eic::RawCALOROCHitCollection& coll,
    uint64_t cellID,
    int32_t timeStamp,
    int32_t samplePhase,
    uint16_t highGainADC,
    uint16_t lowGainADC,
    uint16_t timeOfArrival)
{
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
static edm4hep::MutableSimCalorimeterHit make_npe_hit(
    edm4hep::SimCalorimeterHitCollection& hitColl,
    edm4hep::CaloHitContributionCollection& contribColl,
    uint64_t cellID,
    edm4hep::Vector3f position,
    std::initializer_list<float> contrib_energies)
{
  float total = 0.f;
  for (auto e : contrib_energies) total += e;
  auto hit = hitColl.create();
  hit.setCellID(cellID);
  hit.setEnergy(total);
  hit.setPosition(position);
  for (auto e : contrib_energies) {
    auto c = contribColl.create();
    c.setEnergy(e);
    c.setTime(0.f);
    c.setStepPosition({0.f, 0.f, 0.f});
    hit.addToContributions(c);
  }
  return hit;
}

// Write a minimal LUT file with one entry: layer=0 sector=0 -> factor=1.0
// Returns the file path as a string
static std::string write_lut_file(const std::string& path) {
  std::ofstream f(path);
  f << "0 0 1.0\n";
  return path;
}

// Convenience: build the standard config used in most tests
static eicrecon::CalorimeterCALOROCCalibrationConfig make_cfg(const std::string& lut_path) {
  eicrecon::CalorimeterCALOROCCalibrationConfig cfg;
  cfg.readout              = "MockCALOROCHits";
  cfg.layerField           = "layer";
  cfg.sectorField          = "sector";
  cfg.localDetFields       = {"system"};
  cfg.edep_to_npe_filename = lut_path;
  cfg.edep_to_npe_fields   = {"layer", "sector"};

  // Identity-like attenuation: factor ≈ 1 for all reasonable z
  cfg.attenuationParameters = {1.0, 1.0e9, 1.0e9};

  cfg.attenuationReferencePositionNamePos = "MockCALOROC_RefPosP";
  cfg.attenuationReferencePositionNameNeg = "MockCALOROC_RefPosN";

  cfg.slope     = 1.0;
  cfg.intercept = 0.0;

  cfg.highGainDR = 1000; // saturation threshold
  cfg.gainRatio  = 1.0;  // high/low gain ratio = 1 for simplicity

  // Light speed parameters: zpos = dt * 1.0 + 0.0
  cfg.lightSpeedParameters = {1.0, 0.0};

  cfg.timeWalkCor = false; // disabled by default; enabled per test-case
  cfg.timeWalkCorrectionParameters = {0.0, 1.0, 0.0, 1.0};
  // correction = toa - (1.0 * (ADCsum - 0)^1 + 0) = toa - ADCsum

  cfg.useNpeHitPos = false; // use timing-based z by default
  cfg.proxy_type   = eicrecon::CalorimeterCALOROCCalibrationConfig::ProxyType::sum;

  return cfg;
}

// ── TOA formula (for expected-value computation in tests) ──────────────────────
// toa = samplePhase*(25/1042) + (timeStamp + idx_toa)*25 - timeOfArrival*(25/1024)
static double expected_toa(int samplePhase, int timeStamp, int idx_toa, uint16_t toa_val) {
  return samplePhase * (25.0 / 1042.0)
       + (timeStamp + idx_toa) * 25.0
       - toa_val * (25.0 / 1024.0);
}

// ────────────────────────────────────────────────────────────────────────────────
// Test 1: Amplitude from ADC sum (no saturation)
// ────────────────────────────────────────────────────────────────────────────────
TEST_CASE("CalorimeterCALOROCCalibration: ADC amplitude in low-signal regime",
          "[CalorimeterCALOROCCalibration][Amplitude]") {

  // Write LUT
  const std::string lut_path = "/tmp/caloroc_test_lut_amplitude.txt";
  write_lut_file(lut_path);

  auto cfg = make_cfg(lut_path);

  eicrecon::CalorimeterCALOROCCalibration algo("CALOROCCalibrationAmplitude");
  algo.applyConfig(cfg);
  algo.init();

  auto detector = algorithms::GeoSvc::instance().detector();
  auto id_desc  = detector->readout("MockCALOROCHits").idSpec();

  // cellID with layer=0, sector=0 (matches LUT entry)
   uint64_t cellID = id_desc.encode({{"system", 4}, {"layer", 0}, {"sector", 0}, {"x", 0}, {"y", 0}});

   // P-side: highGainADC=100, below highGainDR=1000 -> npeP = 100/1.0 = 100
   // N-side: highGainADC=200 -> npeN = 200
   edm4eic::RawCALOROCHitCollection adcP_coll, adcN_coll;
   // Symmetric timing on both sides: tP = tN = -12.5, dt = 0, zpos = 0.
   // Timing is irrelevant for this test; both sides use identical timeStamp=0, toa=512.
   make_raw_hit(adcP_coll, cellID, /*timeStamp=*/0, /*samplePhase=*/0,
                /*highGainADC=*/100, /*lowGainADC=*/50, /*timeOfArrival=*/512);
   make_raw_hit(adcN_coll, cellID, /*timeStamp=*/0, /*samplePhase=*/0,
                /*highGainADC=*/200, /*lowGainADC=*/100, /*timeOfArrival=*/512);

   edm4hep::SimCalorimeterHitCollection npeP_coll, npeN_coll;
   edm4hep::CaloHitContributionCollection contribs_coll;
   make_npe_hit(npeP_coll, contribs_coll, cellID, {0.f, 0.f, 0.f}, {60.f});
   make_npe_hit(npeN_coll, contribs_coll, cellID, {0.f, 0.f, 0.f}, {40.f});

  auto recohits  = std::make_unique<edm4eic::CalorimeterHitCollection>();
  auto rawhits   = std::make_unique<edm4hep::RawCalorimeterHitCollection>();
  edm4eic::MCRecoCalorimeterHitLinkCollection        links;
  edm4eic::MCRecoCalorimeterHitAssociationCollection assocs;

   algo.process(
       {&npeP_coll, &adcP_coll, &npeN_coll, &adcN_coll},
       {recohits.get(), rawhits.get(), &links, &assocs});

    const int64_t actual_amplitude = (*rawhits)[0].getAmplitude();
    std::cout << "--- ADC amplitude (no saturation) ---" << "\n";
    std::cout << "P-side highGainADC = 100, gainRatio = 1.0" << "\n";
    std::cout << "Expected amplitude (npeP = highGainADC / gainRatio) = 100" << "\n";
    std::cout << "Number of rawhits produced = " << rawhits->size() << "\n";
    std::cout << "Actual rawhit amplitude    = " << actual_amplitude << "\n";
    REQUIRE(rawhits->size() == 1);
    REQUIRE(actual_amplitude == 100);
}

// ────────────────────────────────────────────────────────────────────────────────
// Test 2: Amplitude uses lowGainADC when highGain saturates
// ────────────────────────────────────────────────────────────────────────────────
TEST_CASE("CalorimeterCALOROCCalibration: ADC amplitude switches to low-gain on saturation",
          "[CalorimeterCALOROCCalibration][Amplitude]") {

  const std::string lut_path = "/tmp/caloroc_test_lut_saturation.txt";
  write_lut_file(lut_path);

  auto cfg = make_cfg(lut_path);

  eicrecon::CalorimeterCALOROCCalibration algo("CALOROCCalibrationSaturation");
  algo.applyConfig(cfg);
  algo.init();

  auto detector = algorithms::GeoSvc::instance().detector();
  auto id_desc  = detector->readout("MockCALOROCHits").idSpec();
  uint64_t cellID = id_desc.encode({{"system", 4}, {"layer", 0}, {"sector", 0}, {"x", 0}, {"y", 0}});

   edm4eic::RawCALOROCHitCollection adcP_coll, adcN_coll;
   // Symmetric timing on both sides: timing is not under test here.
   // P-side: highGainADC=1000 >= highGainDR=1000 -> use lowGainADC=75 directly
   make_raw_hit(adcP_coll, cellID, 0, 0, /*highGainADC=*/1000, /*lowGainADC=*/75, /*toa=*/512);
    // N-side: also saturated -> lowGainADC=150
    make_raw_hit(adcN_coll, cellID, 0, 0, /*highGainADC=*/1000, /*lowGainADC=*/150, /*toa=*/512);

   edm4hep::SimCalorimeterHitCollection npeP_coll, npeN_coll;
   edm4hep::CaloHitContributionCollection contribs_coll;
   make_npe_hit(npeP_coll, contribs_coll, cellID, {0.f, 0.f, 0.f}, {60.f});
   make_npe_hit(npeN_coll, contribs_coll, cellID, {0.f, 0.f, 0.f}, {40.f});

  auto recohits  = std::make_unique<edm4eic::CalorimeterHitCollection>();
  auto rawhits   = std::make_unique<edm4hep::RawCalorimeterHitCollection>();
  edm4eic::MCRecoCalorimeterHitLinkCollection        links;
  edm4eic::MCRecoCalorimeterHitAssociationCollection assocs;

   algo.process(
       {&npeP_coll, &adcP_coll, &npeN_coll, &adcN_coll},
       {recohits.get(), rawhits.get(), &links, &assocs});

    const int64_t actual_amplitude = (*rawhits)[0].getAmplitude();
    std::cout << "--- ADC amplitude (high-gain saturation) ---" << "\n";
    std::cout << "P-side highGainADC = 1000 >= highGainDR = 1000 -> switch to lowGainADC = 75" << "\n";
    std::cout << "Expected amplitude = 75" << "\n";
    std::cout << "Number of rawhits produced = " << rawhits->size() << "\n";
    std::cout << "Actual rawhit amplitude    = " << actual_amplitude << "\n";
    REQUIRE(rawhits->size() == 1);
    REQUIRE(actual_amplitude == 75);
}

// ────────────────────────────────────────────────────────────────────────────────
// Test 3: z-position from delta-t
// ────────────────────────────────────────────────────────────────────────────────
TEST_CASE("CalorimeterCALOROCCalibration: z-position from timing difference",
          "[CalorimeterCALOROCCalibration][ZPosition]") {

  const std::string lut_path = "/tmp/caloroc_test_lut_zpos.txt";
  write_lut_file(lut_path);

  auto cfg = make_cfg(lut_path);
  cfg.timeWalkCor  = false;
  cfg.useNpeHitPos = false;

  eicrecon::CalorimeterCALOROCCalibration algo("CALOROCCalibrationZPos");
  algo.applyConfig(cfg);
  algo.init();

  auto detector = algorithms::GeoSvc::instance().detector();
  auto id_desc  = detector->readout("MockCALOROCHits").idSpec();
  uint64_t cellID = id_desc.encode({{"system", 4}, {"layer", 0}, {"sector", 0}, {"x", 0}, {"y", 0}});

   // P-side: timeStamp=0, samplePhase=0, toa=512 -> tP = 0 + 0*25 - 512*(25/1024) = -12.5
   // N-side: timeStamp=1, samplePhase=0, toa=512 -> tN = 0 + 1*25 - 12.5 = 12.5
   // dt = tN - tP = 12.5 - (-12.5) = 25.0
   // zpos = 25.0 * 1.0 + 0.0 = 25.0
   edm4eic::RawCALOROCHitCollection adcP_coll, adcN_coll;
   make_raw_hit(adcP_coll, cellID, 0, 0, 100, 50, 512);
   make_raw_hit(adcN_coll, cellID, 1, 0, 200, 100, 512);

   edm4hep::SimCalorimeterHitCollection npeP_coll, npeN_coll;
   edm4hep::CaloHitContributionCollection contribs_coll;
   make_npe_hit(npeP_coll, contribs_coll, cellID, {0.f, 0.f, 0.f}, {60.f});
   make_npe_hit(npeN_coll, contribs_coll, cellID, {0.f, 0.f, 0.f}, {40.f});

  auto recohits  = std::make_unique<edm4eic::CalorimeterHitCollection>();
  auto rawhits   = std::make_unique<edm4hep::RawCalorimeterHitCollection>();
  edm4eic::MCRecoCalorimeterHitLinkCollection        links;
  edm4eic::MCRecoCalorimeterHitAssociationCollection assocs;

   algo.process(
       {&npeP_coll, &adcP_coll, &npeN_coll, &adcN_coll},
       {recohits.get(), rawhits.get(), &links, &assocs});

    const double tP_expected   = expected_toa(0, 0, 0, 512); // -12.5
    const double tN_expected   = expected_toa(0, 1, 0, 512); //  12.5
    const double dt_expected   = tN_expected - tP_expected;  //  25.0
    const double zpos_expected = dt_expected * 1.0 + 0.0;    //  25.0
    const double actual_z      = (*recohits)[0].getPosition().z;
    std::cout << "--- z-position from timing difference ---" << "\n";
    std::cout << "P-side: timeStamp=0, samplePhase=0, timeOfArrival=512" << "\n";
    std::cout << "N-side: timeStamp=1, samplePhase=0, timeOfArrival=512" << "\n";
    std::cout << "tP (expected)   = " << tP_expected << " ns" << "\n";
    std::cout << "tN (expected)   = " << tN_expected << " ns" << "\n";
    std::cout << "dt = tN - tP    = " << dt_expected << " ns" << "\n";
    std::cout << "zpos = dt * lightSpeed[0] + lightSpeed[1] = " << zpos_expected << " mm" << "\n";
    std::cout << "Number of recohits = " << recohits->size() << "\n";
    std::cout << "Actual recohit z   = " << actual_z << " mm" << "\n";
    REQUIRE(recohits->size() == 1);
    REQUIRE_THAT(actual_z, Catch::Matchers::WithinAbs(zpos_expected, 1e-4));
}

// ────────────────────────────────────────────────────────────────────────────────
// Test 4: z-position from NpeHit (useNpeHitPos=true)
// ────────────────────────────────────────────────────────────────────────────────
TEST_CASE("CalorimeterCALOROCCalibration: z-position read from NpeHit position",
          "[CalorimeterCALOROCCalibration][ZPosition]") {

  const std::string lut_path = "/tmp/caloroc_test_lut_npepos.txt";
  write_lut_file(lut_path);

  auto cfg = make_cfg(lut_path);
  cfg.useNpeHitPos = true;

  eicrecon::CalorimeterCALOROCCalibration algo("CALOROCCalibrationNpePos");
  algo.applyConfig(cfg);
  algo.init();

  auto detector = algorithms::GeoSvc::instance().detector();
  auto id_desc  = detector->readout("MockCALOROCHits").idSpec();
  uint64_t cellID = id_desc.encode({{"system", 4}, {"layer", 0}, {"sector", 0}, {"x", 0}, {"y", 0}});

   edm4eic::RawCALOROCHitCollection adcP_coll, adcN_coll;
   make_raw_hit(adcP_coll, cellID, 0, 0, 100, 50, 512);
   make_raw_hit(adcN_coll, cellID, 1, 0, 200, 100, 512);

   edm4hep::SimCalorimeterHitCollection npeP_coll, npeN_coll;
   edm4hep::CaloHitContributionCollection contribs_coll;
   // NpeHitP has z = 42.0 mm — this should be used as zpos
   make_npe_hit(npeP_coll, contribs_coll, cellID, {0.f, 0.f, 42.f}, {60.f});
   make_npe_hit(npeN_coll, contribs_coll, cellID, {0.f, 0.f,  0.f}, {40.f});

  auto recohits  = std::make_unique<edm4eic::CalorimeterHitCollection>();
  auto rawhits   = std::make_unique<edm4hep::RawCalorimeterHitCollection>();
  edm4eic::MCRecoCalorimeterHitLinkCollection        links;
  edm4eic::MCRecoCalorimeterHitAssociationCollection assocs;

   algo.process(
       {&npeP_coll, &adcP_coll, &npeN_coll, &adcN_coll},
       {recohits.get(), rawhits.get(), &links, &assocs});

    const double npe_z    = 42.0;
    const double actual_z = (*recohits)[0].getPosition().z;
    std::cout << "--- z-position from NpeHit position (useNpeHitPos=true) ---" << "\n";
    std::cout << "NpeHitP position.z = " << npe_z << " mm  (set explicitly)" << "\n";
    std::cout << "Expected recohit z = " << npe_z << " mm" << "\n";
    std::cout << "Number of recohits = " << recohits->size() << "\n";
    std::cout << "Actual recohit z   = " << actual_z << " mm" << "\n";
    REQUIRE(recohits->size() == 1);
    REQUIRE_THAT(actual_z, Catch::Matchers::WithinAbs(npe_z, 1e-4));
}

// ────────────────────────────────────────────────────────────────────────────────
// Test 5: Time walk correction shifts the reconstructed z
// ────────────────────────────────────────────────────────────────────────────────
TEST_CASE("CalorimeterCALOROCCalibration: time walk correction changes z from delta-t",
          "[CalorimeterCALOROCCalibration][TimeWalk]") {

  const std::string lut_path = "/tmp/caloroc_test_lut_timewalk.txt";
  write_lut_file(lut_path);

  auto detector = algorithms::GeoSvc::instance().detector();
  auto id_desc  = detector->readout("MockCALOROCHits").idSpec();
  uint64_t cellID = id_desc.encode({{"system", 4}, {"layer", 0}, {"sector", 0}, {"x", 0}, {"y", 0}});

  edm4eic::RawCALOROCHitCollection adcP_coll, adcN_coll;
  // P-side ADC sum = 100 (highGainADC=100 < 1000, gainRatio=1)
  // N-side ADC sum = 200
  make_raw_hit(adcP_coll, cellID, 0, 0, 100, 50, 512);
  make_raw_hit(adcN_coll, cellID, 1, 0, 200, 100, 512);

  edm4hep::SimCalorimeterHitCollection npeP_coll, npeN_coll;
  edm4hep::CaloHitContributionCollection contribs_coll;
  make_npe_hit(npeP_coll, contribs_coll, cellID, {0.f, 0.f, 0.f}, {60.f});
  make_npe_hit(npeN_coll, contribs_coll, cellID, {0.f, 0.f, 0.f}, {40.f});

   // Without time walk correction:
   // tP = -12.5, tN = 12.5, dt = 25.0, zpos = 25.0
   auto cfg_nocor = make_cfg(lut_path);
   cfg_nocor.timeWalkCor = false;
   {
     eicrecon::CalorimeterCALOROCCalibration algo("CALOROCCalibrationNoTWC");
     algo.applyConfig(cfg_nocor);
     algo.init();

     auto recohits = std::make_unique<edm4eic::CalorimeterHitCollection>();
     auto rawhits  = std::make_unique<edm4hep::RawCalorimeterHitCollection>();
     edm4eic::MCRecoCalorimeterHitLinkCollection        links;
     edm4eic::MCRecoCalorimeterHitAssociationCollection assocs;
     algo.process({&npeP_coll, &adcP_coll, &npeN_coll, &adcN_coll},
                  {recohits.get(), rawhits.get(), &links, &assocs});

      const double tP_raw      = expected_toa(0, 0, 0, 512); // -12.5
      const double tN_raw      = expected_toa(0, 1, 0, 512); //  12.5
      const double dt_raw      = tN_raw - tP_raw;
      const double zpos_raw    = dt_raw * 1.0 + 0.0;
      const double actual_z    = (*recohits)[0].getPosition().z;
      std::cout << "--- Time walk correction OFF ---" << "\n";
      std::cout << "tP (raw)         = " << tP_raw << " ns" << "\n";
      std::cout << "tN (raw)         = " << tN_raw << " ns" << "\n";
      std::cout << "dt (raw)         = " << dt_raw << " ns" << "\n";
      std::cout << "Expected zpos    = " << zpos_raw << " mm" << "\n";
      std::cout << "Actual recohit z = " << actual_z << " mm" << "\n";
      REQUIRE(recohits->size() == 1);
      REQUIRE_THAT(actual_z, Catch::Matchers::WithinAbs(25.0, 1e-4));
   }

   // With time walk correction (params = {0, 1, 0, 1}):
   // corrected tP = -12.5 - (1.0*(100-0)^1 + 0) = -112.5
   // corrected tN = 12.5  - (1.0*(200-0)^1 + 0) = -187.5
   // dt_corrected = -187.5 - (-112.5) = -75.0
   // zpos_corrected = -75.0 (within [-1000, 1000], no clamping)
   auto cfg_cor = make_cfg(lut_path);
   cfg_cor.timeWalkCor               = true;
   cfg_cor.timeWalkCorrectionParameters = {0.0, 1.0, 0.0, 1.0};
   {
     eicrecon::CalorimeterCALOROCCalibration algo("CALOROCCalibrationWithTWC");
     algo.applyConfig(cfg_cor);
     algo.init();

     auto recohits = std::make_unique<edm4eic::CalorimeterHitCollection>();
     auto rawhits  = std::make_unique<edm4hep::RawCalorimeterHitCollection>();
     edm4eic::MCRecoCalorimeterHitLinkCollection        links;
     edm4eic::MCRecoCalorimeterHitAssociationCollection assocs;
     algo.process({&npeP_coll, &adcP_coll, &npeN_coll, &adcN_coll},
                  {recohits.get(), rawhits.get(), &links, &assocs});

      const double tP_raw       = expected_toa(0, 0, 0, 512); // -12.5
      const double tN_raw       = expected_toa(0, 1, 0, 512); //  12.5
      const double adcSumP      = 100.0; // highGainADC=100, gainRatio=1
      const double adcSumN      = 200.0; // highGainADC=200, gainRatio=1
      // correction = toa - (p[1] * (ADCsum - p[2])^p[3] + p[0])
      //            = toa - (1.0  * (ADCsum -    0)^1.0  + 0.0)
      //            = toa - ADCsum
      const double tP_corrected = tP_raw - adcSumP; // -12.5 - 100 = -112.5
      const double tN_corrected = tN_raw - adcSumN; //  12.5 - 200 = -187.5
      const double dt_corrected = tN_corrected - tP_corrected; // -75.0
      const double zpos_corrected = dt_corrected * 1.0 + 0.0; // -75.0
      const double actual_z     = (*recohits)[0].getPosition().z;
      std::cout << "--- Time walk correction ON ---" << "\n";
      std::cout << "timeWalkCorrectionParameters = {0, 1, 0, 1}" << "\n";
      std::cout << "tP (raw)         = " << tP_raw        << " ns" << "\n";
      std::cout << "tN (raw)         = " << tN_raw        << " ns" << "\n";
      std::cout << "ADC sum P-side   = " << adcSumP << "\n";
      std::cout << "ADC sum N-side   = " << adcSumN << "\n";
      std::cout << "tP (corrected)   = " << tP_corrected  << " ns" << "\n";
      std::cout << "tN (corrected)   = " << tN_corrected  << " ns" << "\n";
      std::cout << "dt (corrected)   = " << dt_corrected  << " ns" << "\n";
      std::cout << "Expected zpos    = " << zpos_corrected << " mm" << "\n";
      std::cout << "Actual recohit z = " << actual_z       << " mm" << "\n";
      REQUIRE(recohits->size() == 1);
      REQUIRE_THAT(actual_z, Catch::Matchers::WithinAbs(-75.0, 1e-4));
   }
}

// ────────────────────────────────────────────────────────────────────────────────
// Test 6: MC truth links – correct counts and energy-normalized weights
// ────────────────────────────────────────────────────────────────────────────────
TEST_CASE("CalorimeterCALOROCCalibration: MC truth link weights are energy-normalized",
          "[CalorimeterCALOROCCalibration][MCLinks]") {

  const std::string lut_path = "/tmp/caloroc_test_lut_links.txt";
  write_lut_file(lut_path);

  auto cfg = make_cfg(lut_path);

  eicrecon::CalorimeterCALOROCCalibration algo("CALOROCCalibrationLinks");
  algo.applyConfig(cfg);
  algo.init();

  auto detector = algorithms::GeoSvc::instance().detector();
  auto id_desc  = detector->readout("MockCALOROCHits").idSpec();
   uint64_t cellID = id_desc.encode({{"system", 4}, {"layer", 0}, {"sector", 0}, {"x", 0}, {"y", 0}});

   edm4eic::RawCALOROCHitCollection adcP_coll, adcN_coll;
   // Symmetric timing: timing irrelevant here, both sides identical to keep zpos=0.
   make_raw_hit(adcP_coll, cellID, 0, 0, 100, 50, 512);
   make_raw_hit(adcN_coll, cellID, 0, 0, 200, 100, 512);

    // NpeHitP has 2 contributions: 30 and 70 -> total 100
    // NpeHitN has 1 contribution: 50
    // Total edep = 30 + 70 + 50 = 150
   edm4hep::SimCalorimeterHitCollection npeP_coll, npeN_coll;
   edm4hep::CaloHitContributionCollection contribs_coll;
   make_npe_hit(npeP_coll, contribs_coll, cellID, {0.f, 0.f, 0.f}, {30.f, 70.f});
   make_npe_hit(npeN_coll, contribs_coll, cellID, {0.f, 0.f, 0.f}, {50.f});

  auto recohits  = std::make_unique<edm4eic::CalorimeterHitCollection>();
  auto rawhits   = std::make_unique<edm4hep::RawCalorimeterHitCollection>();
  edm4eic::MCRecoCalorimeterHitLinkCollection        links;
  edm4eic::MCRecoCalorimeterHitAssociationCollection assocs;

   algo.process(
       {&npeP_coll, &adcP_coll, &npeN_coll, &adcN_coll},
       {recohits.get(), rawhits.get(), &links, &assocs});

    const double edepP1     = 30.0, edepP2 = 70.0, edepN1 = 50.0;
    const double total_edep = edepP1 + edepP2 + edepN1;
    std::vector<double> weights;
    for (const auto& lnk : links) weights.push_back(lnk.getWeight());
    std::sort(weights.begin(), weights.end());
    double total_weight = 0.;
    for (auto w : weights) total_weight += w;

    std::cout << "--- MC truth link weights ---" << "\n";
    std::cout << "NpeHitP contributions: " << edepP1 << ", " << edepP2 << "\n";
    std::cout << "NpeHitN contributions: " << edepN1 << "\n";
    std::cout << "Total edep = " << total_edep << "\n";
    std::cout << "Expected weights (sorted): "
         << edepP1/total_edep << ", " << edepN1/total_edep << ", " << edepP2/total_edep << "\n";
    std::cout << "Number of links  = " << links.size() << "\n";
    std::cout << "Number of assocs = " << assocs.size() << "\n";
    std::cout << "Sum of weights   = " << total_weight << "\n";
    if (weights.size() == 3) {
      std::cout << "Actual weights (sorted): " << weights[0] << ", " << weights[1] << ", " << weights[2] << "\n";
    }

    REQUIRE(links.size() == 3);
    REQUIRE(assocs.size() == 3);
    REQUIRE_THAT(total_weight, Catch::Matchers::WithinAbs(1.0, 1e-5));
    REQUIRE_THAT(weights[0], Catch::Matchers::WithinAbs(30.0 / 150.0, 1e-5));
    REQUIRE_THAT(weights[1], Catch::Matchers::WithinAbs(50.0 / 150.0, 1e-5));
    REQUIRE_THAT(weights[2], Catch::Matchers::WithinAbs(70.0 / 150.0, 1e-5));
}

// ────────────────────────────────────────────────────────────────────────────────
// Test 7: Average time stored in rawhit timestamp
// ────────────────────────────────────────────────────────────────────────────────
TEST_CASE("CalorimeterCALOROCCalibration: rawhit timestamp is average of P and N TOA",
          "[CalorimeterCALOROCCalibration][Timing]") {

  const std::string lut_path = "/tmp/caloroc_test_lut_time.txt";
  write_lut_file(lut_path);

  auto cfg = make_cfg(lut_path);
  cfg.timeWalkCor = false;

  eicrecon::CalorimeterCALOROCCalibration algo("CALOROCCalibrationTime");
  algo.applyConfig(cfg);
  algo.init();

  auto detector = algorithms::GeoSvc::instance().detector();
  auto id_desc  = detector->readout("MockCALOROCHits").idSpec();
  uint64_t cellID = id_desc.encode({{"system", 4}, {"layer", 0}, {"sector", 0}, {"x", 0}, {"y", 0}});

   // tP = expected_toa(0, 0, 0, 512) = -12.5
   // tN = expected_toa(0, 1, 0, 512) =  12.5
   // time = 0.5 * (-12.5 + 12.5) = 0.0
   edm4eic::RawCALOROCHitCollection adcP_coll, adcN_coll;
   make_raw_hit(adcP_coll, cellID, 0, 0, 100, 50, 512);
   make_raw_hit(adcN_coll, cellID, 1, 0, 200, 100, 512);

   edm4hep::SimCalorimeterHitCollection npeP_coll, npeN_coll;
   edm4hep::CaloHitContributionCollection contribs_coll;
   make_npe_hit(npeP_coll, contribs_coll, cellID, {0.f, 0.f, 0.f}, {60.f});
   make_npe_hit(npeN_coll, contribs_coll, cellID, {0.f, 0.f, 0.f}, {40.f});

  auto recohits  = std::make_unique<edm4eic::CalorimeterHitCollection>();
  auto rawhits   = std::make_unique<edm4hep::RawCalorimeterHitCollection>();
  edm4eic::MCRecoCalorimeterHitLinkCollection        links;
  edm4eic::MCRecoCalorimeterHitAssociationCollection assocs;

   algo.process(
       {&npeP_coll, &adcP_coll, &npeN_coll, &adcN_coll},
       {recohits.get(), rawhits.get(), &links, &assocs});

    const double tP          = expected_toa(0, 0, 0, 512);
    const double tN          = expected_toa(0, 1, 0, 512);
    const double expected    = 0.5 * (tP + tN);
    const double actual_time = static_cast<double>((*rawhits)[0].getTimeStamp());
    std::cout << "--- Rawhit timestamp = average of P and N TOA ---" << "\n";
    std::cout << "P-side: timeStamp=0, samplePhase=0, timeOfArrival=512" << "\n";
    std::cout << "N-side: timeStamp=1, samplePhase=0, timeOfArrival=512" << "\n";
    std::cout << "tP (formula)       = " << tP       << " ns" << "\n";
    std::cout << "tN (formula)       = " << tN       << " ns" << "\n";
    std::cout << "Expected timestamp = 0.5*(tP+tN) = " << expected   << " ns" << "\n";
    std::cout << "Number of rawhits  = " << rawhits->size() << "\n";
    std::cout << "Actual timestamp   = " << actual_time << " ns" << "\n";
    REQUIRE(rawhits->size() == 1);
    REQUIRE_THAT(actual_time, Catch::Matchers::WithinAbs(expected, 1e-4));
}

#endif // EDM4EIC_VERSION_MAJOR
