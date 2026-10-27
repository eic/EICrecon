// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Aiden Wu

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <algorithms/geo.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <edm4eic/CALOROC1ASample.h>
#include <edm4eic/CALOROC1BSample.h>
#include <edm4eic/unit_system.h>
#include <cstdint>
#include <stdexcept>

#include "algorithms/calorimetry/CalorimeterCALOROCSingleEndedReco.h"

TEST_CASE("CalorimeterCALOROCSingleEndedReco: single-ended per-pulse reconstruction",
          "[CalorimeterCALOROCSingleEndedReco][SingleEnded]") {
  eicrecon::CalorimeterCALOROCSingleEndedRecoConfig cfg;
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
  SECTION("Hits without a measured TOA use the source pulse time") { toa = 0; }
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
  SECTION("Dual-gain samples switch to low gain upon saturation") {
    cfg.calorocType               = "1B";
    cfg.caloroc.dyRangeLowGainADC = 10;
    expected_energy               = 230 * edm4eic::unit::MeV;
  }

  SECTION("Dual-gain pedestal subtraction uses single-gain ADC units") {
    cfg.calorocType                  = "1B";
    cfg.caloroc.dyRangeHighGainADC   = 2;
    cfg.caloroc.dyRangeLowGainADC    = 20;
    cfg.caloroc.dyRangeSingleGainADC = 4;
    expected_energy                  = 105 * edm4eic::unit::MeV;
  }
  SECTION("Dual-gain conversion with zero pedestal") {
    cfg.calorocType               = "1B";
    cfg.caloroc.dyRangeLowGainADC = 10;
    cfg.pedestalADC               = 0;
    expected_energy               = 250 * edm4eic::unit::MeV;
  }
  SECTION("Time walk uses the gain-selected single-gain ADC sum") {
    cfg.calorocType                  = "1B";
    cfg.caloroc.dyRangeLowGainADC    = 10;
    cfg.timeWalkCor                  = true;
    cfg.timeWalkCorrectionParameters = {0, 0.01, 0, 1};
    expected_energy                  = 230 * edm4eic::unit::MeV;
    expected_time_correction         = 2.3;
  }

  eicrecon::CalorimeterCALOROCSingleEndedReco algo("CALOROCRecoSingleEnded");
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
    if (empty_samples)
      continue;
    hit.addToASamples({adc, toa, tot});
    hit.addToASamples({8, 0, 0});
    edm4eic::CALOROC1BSample high_gain;
    high_gain.highGainADC   = 50;
    high_gain.lowGainADC    = 15;
    high_gain.timeOfArrival = toa;
    hit.addToBSamples(high_gain);
    edm4eic::CALOROC1BSample low_gain;
    low_gain.highGainADC = 1023;
    low_gain.lowGainADC  = 20;
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
    REQUIRE_THAT(reco_hits[i].getEnergy(),
                 Catch::Matchers::WithinAbs(expected_energy, 1e-6 * edm4eic::unit::MeV));
    const double expected_time = (toa == 0 ? 80.0 : 87.5) + 100 * i - expected_time_correction;
    REQUIRE_THAT(
        reco_hits[i].getTime(),
        Catch::Matchers::WithinAbs(expected_time * edm4eic::unit::ns, 1e-6 * edm4eic::unit::ns));
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
      const double weight = combined_truth ? (j == 0 ? 0.25 : 0.75) : 1;
      REQUIRE_THAT(links[index].getWeight(), Catch::Matchers::WithinAbs(weight, 1e-6));
      REQUIRE_THAT(associations[index].getWeight(), Catch::Matchers::WithinAbs(weight, 1e-6));
    }
  }
}

TEST_CASE("CalorimeterCALOROCSingleEndedReco: time-walk coefficient count",
          "[CalorimeterCALOROCSingleEndedReco][SingleEnded]") {
  eicrecon::CalorimeterCALOROCSingleEndedRecoConfig cfg;
  cfg.timeWalkCor                  = true;
  cfg.timeWalkCorrectionParameters = {0, 1};
  eicrecon::CalorimeterCALOROCSingleEndedReco algo("CALOROCRecoSingleEndedInvalidTimeWalk");
  algo.applyConfig(cfg);
  REQUIRE_THROWS_AS(algo.init(), std::runtime_error);
}

TEST_CASE("CalorimeterCALOROCSingleEndedReco: ToT coefficient count",
          "[CalorimeterCALOROCSingleEndedReco][SingleEnded]") {
  eicrecon::CalorimeterCALOROCSingleEndedRecoConfig cfg;
  cfg.totEnergyParameters = {0, 1};
  eicrecon::CalorimeterCALOROCSingleEndedReco algo("CALOROCRecoSingleEndedInvalidToT");
  algo.applyConfig(cfg);
  REQUIRE_THROWS_AS(algo.init(), std::runtime_error);
}
