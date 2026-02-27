// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Minho Kim

#include <catch2/catch_test_macros.hpp>
#include <edm4eic/EDM4eicVersion.h>

#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 7)
#include <edm4eic/RawCALOROCHitCollection.h>
#include <edm4eic/SimPulseCollection.h>
#include <edm4eic/unit_system.h>
#include <cmath>
#include <tuple>

#include "algorithms/digi/CALOROCDigitization.h"
#include "algorithms/digi/CALOROCDigitizationConfig.h"

TEST_CASE("Test TOA calculation", "[CALOROCDigitization][TOACalculation]") {

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
    uint16_t TOA_expected = 23 - i;

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
#endif
