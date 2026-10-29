// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Chun Yuen Tsang, Minho Kim, Aiden Wu

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <algorithms/geo.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <edm4eic/CALOROC1BSample.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/MCRecoCalorimeterHitAssociationCollection.h>
#include <edm4eic/MCRecoCalorimeterHitLinkCollection.h>
#include <edm4eic/RawCALOROCHitCollection.h>
#include <edm4eic/SimPulseCollection.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/CaloHitContributionCollection.h>
#include <edm4hep/MCParticle.h>
#include <edm4hep/RawCalorimeterHitCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/Vector3f.h>
#include <podio/ObjectID.h>
#include <podio/detail/Link.h>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <fstream>
#include <gsl/pointers>
#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/calorimetry/CalorimeterCALOROCReco.h"
#include "algorithms/calorimetry/CalorimeterCALOROCRecoConfig.h"
#include "algorithms/digi/CALOROCDigitizationConfig.h"

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

  float total = 0.F;
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
    c.setTime(0.F);
    c.setStepPosition({0.F, 0.F, 0.F});
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
  cfg.calorocType          = "1B";
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
  make_npe_hit(npe_coll, contribs_coll, cellID, {0.F, 0.F, 0.F}, {60.F});

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
  make_raw_hit(adc_coll, cellID, 0, 0, /*highGainADC=*/1000, /*lowGainADC=*/75,
               /*timeOfArrival=*/512);

  edm4hep::SimCalorimeterHitCollection npe_coll;
  edm4hep::CaloHitContributionCollection contribs_coll;
  make_npe_hit(npe_coll, contribs_coll, cellID, {0.F, 0.F, 0.F}, {60.F});

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
  make_npe_hit(npe_coll, contribs_coll, cellID, {0.F, 0.F, 0.F}, {60.F});

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
  make_npe_hit(npe_coll, contribs_coll, cellID, {0.F, 0.F, 0.F}, {60.F});

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
  make_npe_hit(npe_coll, contribs_coll, cellID, {0.F, 0.F, 0.F}, {60.F});

  auto recohits = std::make_unique<edm4eic::CalorimeterHitCollection>();
  auto rawhits  = std::make_unique<edm4hep::RawCalorimeterHitCollection>();

  algo.process({&npe_coll, &adc_coll}, {recohits.get(), rawhits.get()});

  REQUIRE(recohits->empty());
  REQUIRE(rawhits->empty());
}

// ────────────────────────────────────────────────────────────────────────────────
// Test 6: Configurable arrival-time decoding
// ────────────────────────────────────────────────────────────────────────────────
TEST_CASE("CalorimeterCALOROCReco: configurable arrival-time decoding",
          "[CalorimeterCALOROCReco][Timing]") {

  const std::string lut_path =
      std::filesystem::temp_directory_path() / "caloroc_test_lut_configured_timing.txt";
  write_lut_file(lut_path);
  auto cfg                 = make_cfg(lut_path);
  cfg.caloroc.time_window  = 40 * edm4eic::unit::ns;
  cfg.caloroc.capTOA       = 200;
  cfg.caloroc.dyRangeTOA   = 20 * edm4eic::unit::ns;
  std::uint16_t first_toa  = 100;
  std::uint16_t second_toa = 0;
  double expected_time     = 115 * edm4eic::unit::ns;

  SECTION("First sample has an arrival") {}
  SECTION("Later sample has an arrival") {
    first_toa     = 0;
    second_toa    = 100;
    expected_time = 155 * edm4eic::unit::ns;
  }
  SECTION("Missing arrival uses the sample-window end") {
    first_toa     = 0;
    expected_time = 205 * edm4eic::unit::ns;
  }

  const auto cellID = make_cell_id();
  edm4eic::RawCALOROCHitCollection adc_hits;
  auto hit = make_raw_hit(adc_hits, cellID, 3, 50, 100, 50, first_toa);
  edm4eic::CALOROC1BSample sample;
  sample.highGainADC   = 50;
  sample.lowGainADC    = 25;
  sample.timeOfArrival = second_toa;
  hit.addToBSamples(sample);
  edm4hep::SimCalorimeterHitCollection npe_hits;
  edm4hep::CaloHitContributionCollection contributions;
  make_npe_hit(npe_hits, contributions, cellID, {0.F, 0.F, 0.F}, {60.F});
  edm4eic::CalorimeterHitCollection reco_hits;
  edm4hep::RawCalorimeterHitCollection raw_hits;

  eicrecon::CalorimeterCALOROCReco algo("CALOROCRecoConfiguredTiming");
  algo.applyConfig(cfg);
  algo.init();
  algo.process({&npe_hits, &adc_hits}, {&reco_hits, &raw_hits});
  REQUIRE(reco_hits.size() == 1);
  REQUIRE_THAT(reco_hits[0].getTime(), Catch::Matchers::WithinAbs(expected_time, 1e-4));
}

// ────────────────────────────────────────────────────────────────────────────────
// Test 7: Single-ended ADC reconstruction and truth links
// ────────────────────────────────────────────────────────────────────────────────
TEST_CASE("CalorimeterCALOROCReco: ADC single-ended per-pulse reconstruction",
          "[CalorimeterCALOROCReco][SingleEnded]") {

  eicrecon::CalorimeterCALOROCRecoConfig cfg;
  cfg.energyCalibration   = eicrecon::CalorimeterCALOROCRecoConfig::EnergyCalibration::ADC;
  cfg.calorocType         = "1A";
  cfg.readout             = "MockCALOROCHits";
  cfg.layerField          = "layer";
  cfg.pedestalADC         = 10;
  cfg.adcToEnergy         = 0.5 * edm4eic::unit::MeV;
  cfg.sampFrac            = "(layer == 0) ? 0.5 : 0.25";
  cfg.totEnergyParameters = {1 * edm4eic::unit::MeV, 0.2 * edm4eic::unit::MeV / edm4eic::unit::ns,
                             0.01 * edm4eic::unit::MeV / edm4eic::unit::ns / edm4eic::unit::ns};
  std::uint16_t adc       = 14;
  std::uint16_t tot       = 0;
  double expected_energy  = 2 * edm4eic::unit::MeV;
  bool accepted           = true;
  bool empty_samples      = false;
  bool saturated_low_gain = false;
  std::uint16_t toa       = 512;
  int system              = 4;
  int layer               = 0;
  bool combined_truth     = false;
  bool separate_inputs    = false;
  double expected_time_correction = 0;

  SECTION("Pedestal subtraction and sampling correction preserve repeated channel hits") {}
  SECTION("Layer-dependent sampling correction") {
    layer = 1;
    expected_energy *= 2;
  }
  SECTION("Combined pulses preserve original-hit truth and response weights") {
    combined_truth = true;
  }
  SECTION("Per-pulse calls preserve accumulated outputs and source indices") {
    separate_inputs = true;
  }
  SECTION("Empty ADC hits are skipped") {
    empty_samples = true;
    accepted      = false;
  }
  SECTION("Hits without a measured TOA use the sample-window end") { toa = 0; }
  SECTION("Time walk corrects the measured arrival using the pedestal-subtracted sum") {
    cfg.timeWalkCor                  = true;
    cfg.timeWalkCorrectionParameters = {1, 2, 1, 0.5};
    expected_time_correction         = 3;
  }
  SECTION("Disabled time walk preserves the measured arrival") {
    cfg.timeWalkCorrectionParameters = {1, 2, 1, 0.5};
  }
  SECTION("Time walk uses the configured fit at another layer") {
    layer = 1;
    expected_energy *= 2;
    cfg.timeWalkCor                  = true;
    cfg.timeWalkCorrectionParameters = {1, 2, 1, 0.5};
    expected_time_correction         = 3;
  }
  SECTION("Time walk uses independently configured coefficients") {
    cfg.timeWalkCor                  = true;
    cfg.timeWalkCorrectionParameters = {4, 2, 1, 0.5};
    expected_time_correction         = 6;
  }
  SECTION("Time walk preserves arrival times outside the correction domain") {
    cfg.timeWalkCor                  = true;
    cfg.timeWalkCorrectionParameters = {1, 2, 2, 0.5};
  }
  SECTION("Time walk preserves the fallback time without a measured TOA") {
    cfg.timeWalkCor                  = true;
    cfg.timeWalkCorrectionParameters = {1, 2, 1, 0.5};
    toa                              = 0;
  }
  SECTION("Geometry failure propagates") {
    system   = 5;
    accepted = false;
  }
  SECTION("Saturated ADC uses the ToT calibration") {
    adc                              = 1023;
    tot                              = 256;
    const double time_over_threshold = tot / 4096.0 * 200 * edm4eic::unit::ns;
    expected_energy =
        (cfg.totEnergyParameters[0] + cfg.totEnergyParameters[1] * time_over_threshold +
         cfg.totEnergyParameters[2] * time_over_threshold * time_over_threshold) /
        0.5;
  }
  SECTION("Saturated ADC without ToT is rejected") {
    adc      = 1023;
    accepted = false;
  }
  SECTION("ADC-based time walk skips saturated signals") {
    cfg.timeWalkCor                  = true;
    cfg.timeWalkCorrectionParameters = {1, 2, 1, 0.5};
    adc                              = 1023;
    tot                              = 256;
    const double time_over_threshold = tot / 4096.0 * 200 * edm4eic::unit::ns;
    expected_energy =
        (cfg.totEnergyParameters[0] + cfg.totEnergyParameters[1] * time_over_threshold +
         cfg.totEnergyParameters[2] * time_over_threshold * time_over_threshold) /
        0.5;
  }
  SECTION("Clipped ToT rejects the saturated hit") {
    adc      = 1023;
    tot      = 4095;
    accepted = false;
  }

  SECTION("Dual-gain ADC calibration switches to low gain on saturation") {
    cfg.calorocType               = "1B";
    cfg.caloroc.dyRangeLowGainADC = 10;
    expected_energy               = 230 * edm4eic::unit::MeV;
  }
  SECTION("Dual-gain ADC calibration subtracts the single-gain pedestal") {
    cfg.calorocType                  = "1B";
    cfg.caloroc.dyRangeHighGainADC   = 2;
    cfg.caloroc.dyRangeLowGainADC    = 20;
    cfg.caloroc.dyRangeSingleGainADC = 4;
    expected_energy                  = 105 * edm4eic::unit::MeV;
  }
  SECTION("Dual-gain ADC calibration with zero pedestal") {
    cfg.calorocType               = "1B";
    cfg.caloroc.dyRangeLowGainADC = 10;
    cfg.pedestalADC               = 0;
    expected_energy               = 250 * edm4eic::unit::MeV;
  }
  SECTION("Dual-gain ADC calibration rejects saturation of both gains") {
    cfg.calorocType               = "1B";
    cfg.caloroc.dyRangeLowGainADC = 10;
    saturated_low_gain            = true;
    accepted                      = false;
  }
  SECTION("Dual-gain time walk uses the pedestal-subtracted single-gain charge") {
    cfg.calorocType                  = "1B";
    cfg.caloroc.dyRangeLowGainADC    = 10;
    cfg.timeWalkCor                  = true;
    cfg.timeWalkCorrectionParameters = {0, 0.01, 0, 1};
    expected_energy                  = 230 * edm4eic::unit::MeV;
    expected_time_correction         = 2.3;
  }
  SECTION("Dual-gain missing arrival uses the sample-window end") {
    cfg.calorocType               = "1B";
    cfg.caloroc.dyRangeLowGainADC = 10;
    toa                           = 0;
    expected_energy               = 230 * edm4eic::unit::MeV;
  }
  eicrecon::CalorimeterCALOROCReco algo("CALOROCRecoSingleEnded");
  algo.applyConfig(cfg);
  algo.init();
  const auto id_spec = algorithms::GeoSvc::instance().detector()->readout(cfg.readout).idSpec();
  const auto cellID =
      id_spec.encode({{"system", system}, {"layer", layer}, {"sector", 0}, {"x", 0}, {"y", 0}});
  edm4hep::SimCalorimeterHitCollection source_hits;
  edm4hep::SimCalorimeterHitCollection source_subset;
  source_subset.setSubsetCollection(true);
  // A source with zero SiPM response was skipped during conversion.
  source_hits.create().setEnergy(0);
  edm4hep::SimCalorimeterHitCollection sipm_hits;
  edm4eic::SimPulseCollection pulses;
  edm4eic::RawCALOROCHitCollection adc_hits;

  // Loop over two pulses with identical cell IDs and distinct source hits.
  for (unsigned i = 0; i < 2; ++i) {
    auto source = source_hits.create();
    source.setCellID(cellID);
    source.setEnergy((i + 1) * 0.5 * edm4eic::unit::MeV);
    source_subset.push_back(source);
    auto sipm = sipm_hits.create();
    sipm.setCellID(cellID);
    sipm.setEnergy(10); // Converted hits carry fired-pixel counts.
    auto pulse = pulses.create();
    pulse.setCellID(cellID);
    pulse.setTime((80 + 100 * i) * edm4eic::unit::ns);
    pulse.addToCalorimeterHits(sipm);
    if (combined_truth) {
      auto other_source = source_hits.create();
      other_source.setCellID(cellID);
      other_source.setEnergy(2 * edm4eic::unit::MeV);
      source_subset.push_back(other_source);
      auto other_sipm = sipm_hits.create();
      other_sipm.setCellID(cellID);
      other_sipm.setEnergy(30);
      pulse.addToCalorimeterHits(other_sipm);
    }
    auto hit = adc_hits.create();
    hit.setCellID(cellID);
    hit.setTimeStamp(4 + 4 * i);
    hit.setSamplePhase(0);
    if (empty_samples) {
      continue;
    }
    hit.addToASamples({.ADC = adc, .timeOfArrival = toa, .timeOverThreshold = tot});
    hit.addToASamples({.ADC = 8, .timeOfArrival = 0, .timeOverThreshold = 0});
    edm4eic::CALOROC1BSample high_gain;
    high_gain.highGainADC   = 50;
    high_gain.lowGainADC    = 15;
    high_gain.timeOfArrival = toa;
    hit.addToBSamples(high_gain);
    edm4eic::CALOROC1BSample low_gain;
    low_gain.highGainADC   = 1023;
    low_gain.lowGainADC    = saturated_low_gain ? 1023 : 20;
    low_gain.timeOfArrival = 0;
    hit.addToBSamples(low_gain);
  }
  edm4eic::CalorimeterHitCollection reco_hits;
  edm4hep::RawCalorimeterHitCollection raw_hits;
  edm4eic::MCRecoCalorimeterHitLinkCollection links;
  edm4eic::MCRecoCalorimeterHitAssociationCollection associations;
  if (system == 5) {
    REQUIRE_THROWS(algo.process({&adc_hits, &pulses, &source_subset},
                                {&reco_hits, &raw_hits, &links, &associations}));
    return;
  }
  if (separate_inputs) {
    for (unsigned i = 0; i < adc_hits.size(); ++i) {
      edm4eic::RawCALOROCHitCollection input_hits;
      input_hits.setSubsetCollection(true);
      input_hits.push_back(adc_hits[i]);
      edm4eic::SimPulseCollection input_pulses;
      input_pulses.setSubsetCollection(true);
      input_pulses.push_back(pulses[i]);
      algo.process({&input_hits, &input_pulses, &source_subset},
                   {&reco_hits, &raw_hits, &links, &associations});
    }
  } else {
    algo.process({&adc_hits, &pulses, &source_subset},
                 {&reco_hits, &raw_hits, &links, &associations});
  }
  REQUIRE(reco_hits.size() == (accepted ? 2 : 0));
  REQUIRE(raw_hits.size() == reco_hits.size());
  REQUIRE(links.size() == reco_hits.size() * (combined_truth ? 2 : 1));
  REQUIRE(associations.size() == links.size());

  // Loop over reconstructed hits and check calibration, timing, and per-pulse truth.
  for (unsigned i = 0; i < reco_hits.size(); ++i) {
    REQUIRE_THAT(reco_hits[i].getEnergy(), Catch::Matchers::WithinRel(expected_energy, 1e-6));
    const double expected_time = (toa == 0 ? 150.0 : 87.5) + 100 * i - expected_time_correction;
    REQUIRE_THAT(reco_hits[i].getTime(),
                 Catch::Matchers::WithinRel(expected_time * edm4eic::unit::ns, 1e-6));
    REQUIRE(reco_hits[i].getLayer() == layer);
    REQUIRE(reco_hits[i].getSector() == -1);
    REQUIRE(reco_hits[i].getRawHit().getObjectID() == raw_hits[i].getObjectID());
    const unsigned truth_count = combined_truth ? 2 : 1;
    for (unsigned j = 0; j < truth_count; ++j) {
      const auto index  = i * truth_count + j;
      const auto source = source_subset[index];
      REQUIRE(links[index].getFrom().getObjectID() == raw_hits[i].getObjectID());
      REQUIRE(links[index].getTo().getObjectID() == source.getObjectID());
      REQUIRE(links[index].getTo().getEnergy() == source.getEnergy());
      REQUIRE(associations[index].getRawHit().getObjectID() == raw_hits[i].getObjectID());
      REQUIRE(associations[index].getSimHit().getObjectID() == source.getObjectID());
      double weight = 1.0;
      if (combined_truth) {
        weight = j == 0 ? 0.25 : 0.75;
      }
      REQUIRE_THAT(links[index].getWeight(), Catch::Matchers::WithinAbs(weight, 1e-6));
      REQUIRE_THAT(associations[index].getWeight(), Catch::Matchers::WithinAbs(weight, 1e-6));
    }
  }
}

// ────────────────────────────────────────────────────────────────────────────────
// Test 8: ADC time-walk coefficient validation
// ────────────────────────────────────────────────────────────────────────────────
TEST_CASE("CalorimeterCALOROCReco: ADC time-walk coefficient count",
          "[CalorimeterCALOROCReco][SingleEnded]") {

  eicrecon::CalorimeterCALOROCRecoConfig cfg;
  cfg.energyCalibration            = eicrecon::CalorimeterCALOROCRecoConfig::EnergyCalibration::ADC;
  cfg.timeWalkCor                  = true;
  cfg.timeWalkCorrectionParameters = {0, 1};
  eicrecon::CalorimeterCALOROCReco algo("CALOROCRecoSingleEndedInvalidTimeWalk");
  algo.applyConfig(cfg);
  REQUIRE_THROWS_AS(algo.init(), std::runtime_error);
}

// ────────────────────────────────────────────────────────────────────────────────
// Test 9: ADC ToT coefficient validation
// ────────────────────────────────────────────────────────────────────────────────
TEST_CASE("CalorimeterCALOROCReco: ADC ToT coefficient count",
          "[CalorimeterCALOROCReco][SingleEnded]") {

  eicrecon::CalorimeterCALOROCRecoConfig cfg;
  cfg.energyCalibration   = eicrecon::CalorimeterCALOROCRecoConfig::EnergyCalibration::ADC;
  cfg.totEnergyParameters = {0, 1};
  eicrecon::CalorimeterCALOROCReco algo("CALOROCRecoSingleEndedInvalidToT");
  algo.applyConfig(cfg);
  REQUIRE_THROWS_AS(algo.init(), std::runtime_error);
}

// ────────────────────────────────────────────────────────────────────────────────
// Test 10: ADC sample-type validation
// ────────────────────────────────────────────────────────────────────────────────
TEST_CASE("CalorimeterCALOROCReco: invalid sample type", "[CalorimeterCALOROCReco][SingleEnded]") {

  eicrecon::CalorimeterCALOROCRecoConfig cfg;
  cfg.energyCalibration = eicrecon::CalorimeterCALOROCRecoConfig::EnergyCalibration::ADC;
  cfg.calorocType       = "invalid";
  eicrecon::CalorimeterCALOROCReco algo("CALOROCRecoADCInvalidSampleType");
  algo.applyConfig(cfg);
  REQUIRE_THROWS_AS(algo.init(), std::runtime_error);
}
