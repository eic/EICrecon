// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Minho Kim
// Copyright (C) 2026 Aiden Wu

#include <catch2/catch_test_macros.hpp>
#include <podio/RelationRange.h>
#include <cstddef>
#include <cstdint>
#include <memory>

#include <edm4eic/RawCALOROCHitCollection.h>
#include <edm4eic/CALOROC1ASample.h>
#include <edm4eic/SimPulseCollection.h>
#include <edm4eic/unit_system.h>
#include <tuple>

#include "algorithms/digi/CALOROCDigitization.h"
#include "algorithms/digi/CALOROCDigitizationConfig.h"

TEST_CASE("Test TOA calculation", "[CALOROCDigitization][TOACalculation]") {

  // If a pulse has amplitudes, (t, 0), (t+1, 1), and (t+2, 2),
  // and the adc_phase and toa_thres are 0 and 1, respectively,
  // the measured TOA should be 24 - t. This test validates it.

  eicrecon::CALOROCDigitization algo("CALOROCDigitization");
  eicrecon::CALOROCDigitizationConfig cfg;

  cfg.n_samples   = 2;
  cfg.time_window = 25 * edm4eic::unit::ns;
  cfg.adc_phase   = 0 * edm4eic::unit::ns;
  cfg.toa_thres   = 1.;
  cfg.tot_thres   = 1.; // placeholder

  cfg.capADC               = 25;  // placeholder
  cfg.dyRangeSingleGainADC = 25.; // placeholder
  cfg.dyRangeHighGainADC   = 25.; // placeholder
  cfg.dyRangeLowGainADC    = 25.; // placeholder
  cfg.capTOA               = 25.;
  cfg.dyRangeTOA           = 25;
  cfg.capTOT               = 25.; // placeholder
  cfg.dyRangeTOT           = 25;  // placeholder

  algo.applyConfig(cfg);
  algo.init();

  const std::size_t n_tests = 23;
  const std::size_t n_amps  = 3;
  const double pulse_dt     = 1.;

  for (std::size_t i = 0; i < n_tests; i++) {
    std::uint16_t TOA_expected = 23 - i;

    edm4eic::SimPulseCollection pulses;
    auto pulse = pulses.create();
    for (std::size_t j = 0; j < n_amps; j++) {
      pulse.addToAmplitude(j);
    }
    pulse.setCellID(12345); // placeholder
    pulse.setTime(i + 1);
    pulse.setInterval(pulse_dt);
    pulse.setIntegral(2);

    auto digi_hits = std::make_unique<edm4eic::RawCALOROCHitCollection>();

    auto input  = std::make_tuple(&pulses);
    auto output = std::make_tuple(digi_hits.get());

    algo.process(input, output);

    REQUIRE(digi_hits->size() == 1);
    auto a_samples = (*digi_hits)[0].getASamples();
    REQUIRE(a_samples[0].timeOfArrival == TOA_expected);
  }
}

TEST_CASE("Test empty and below-threshold waveforms are skipped", "[CALOROCDigitization][Threshold]") {
  eicrecon::CALOROCDigitization algo("CALOROCDigitization");
  eicrecon::CALOROCDigitizationConfig cfg;
  cfg.toa_thres = 1;
  algo.applyConfig(cfg);
  algo.init();

  edm4eic::SimPulseCollection pulses;
  for (unsigned i = 0; i < 4; ++i) {
    auto pulse = pulses.create();
    pulse.setCellID(12345);
    pulse.setTime(0);
    pulse.setInterval(1 * edm4eic::unit::ns);
    if (i == 0) continue;
    pulse.addToAmplitude(0);
    if (i == 1) continue;
    pulse.addToAmplitude(i == 2 ? 0.5 : 2);
    pulse.addToAmplitude(0);
  }

  edm4eic::RawCALOROCHitCollection digi_hits;
  algo.process(std::make_tuple(&pulses), std::make_tuple(&digi_hits));

  REQUIRE(digi_hits.size() == 1);
  REQUIRE(digi_hits[0].getCellID() == pulses[3].getCellID());
  REQUIRE(digi_hits[0].getASamples().size() == cfg.n_samples);
  REQUIRE(digi_hits[0].getBSamples().size() == cfg.n_samples);
}

TEST_CASE("Test pedestal padding for short waveforms", "[CALOROCDigitization][Pedestal]") {

  // Loop over zero and nonzero pedestal settings.
  for (const unsigned pedestal_adc : {0U, 10U}) {
    eicrecon::CALOROCDigitization algo("CALOROCDigitization");
    eicrecon::CALOROCDigitizationConfig cfg;
    cfg.n_samples = 4;
    cfg.time_window = 1 * edm4eic::unit::ns;
    cfg.toa_thres = 12.0 / 1024;
    cfg.pedestal = pedestal_adc / 1024.0;
    cfg.dyRangeLowGainADC = 10;
    algo.applyConfig(cfg);
    algo.init();

    // Create a short waveform with a measured sample below the pedestal.
    edm4eic::SimPulseCollection pulses;
    auto pulse = pulses.create();
    pulse.setTime(0);
    pulse.setInterval(1 * edm4eic::unit::ns);
    pulse.addToAmplitude(10.0 / 1024);
    pulse.addToAmplitude(14.0 / 1024);
    pulse.addToAmplitude(8.0 / 1024);
    edm4eic::RawCALOROCHitCollection digi_hits;
    algo.process(std::make_tuple(&pulses), std::make_tuple(&digi_hits));

    // Check measured amplitudes and pedestal padding for both chip types.
    REQUIRE(digi_hits.size() == 1);
    const auto a_samples = digi_hits[0].getASamples();
    const auto b_samples = digi_hits[0].getBSamples();
    REQUIRE(a_samples.size() == 4);
    REQUIRE(b_samples.size() == 4);
    REQUIRE(a_samples[0].ADC == 14);
    REQUIRE(a_samples[1].ADC == 8);
    REQUIRE(b_samples[0].highGainADC == 14);
    REQUIRE(b_samples[1].highGainADC == 8);

    // Loop over missing samples.
    for (unsigned i = 2; i < 4; ++i) {
      REQUIRE(a_samples[i].ADC == pedestal_adc);
      REQUIRE(b_samples[i].highGainADC == pedestal_adc);
      REQUIRE(b_samples[i].lowGainADC == pedestal_adc / 10);
    }
  }
}
