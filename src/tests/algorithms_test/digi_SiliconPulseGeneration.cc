// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025, Dmitry Kalinkin, Simon Gardner

#include <algorithms/geo.h>
#include <algorithms/logger.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <edm4hep/SimTrackerHitCollection.h>
#include <edm4hep/TimeSeriesCollection.h>
#include <edm4eic/unit_system.h>
#include <gsl/pointers>
#include <utility>
#include <vector>

#include "algorithms/digi/SiliconPulseGeneration.h"
#include "algorithms/digi/SiliconPulseGenerationConfig.h"

TEST_CASE("SiliconPulseGeneration generates correct number of pulses", "[SiliconPulseGeneration]") {
  eicrecon::SiliconPulseGeneration algo("SiliconPulseGeneration");
  eicrecon::SiliconPulseGenerationConfig cfg;
  cfg.pulse_shape = "LandauPulse"; // Example pulse shape

  algo.applyConfig(cfg);
  algo.init();

  SECTION("from different hits") {
    auto nHits = GENERATE(1, 2, 3);

    edm4hep::SimTrackerHitCollection hits_coll;

    for(int i=0; i<nHits; i++) {
      hits_coll.create(12345 + i, 10.0, 5.0); // cellID, charge, time
    }
  
    edm4hep::TimeSeriesCollection pulses = algo.GeneratePulses(hits_coll);

    REQUIRE(pulses.size() == nHits);
    REQUIRE(pulses[0].getCellID() == 12345);
    if(nHits > 1) {
      REQUIRE(pulses[1].getCellID() == 12346);
    }
    if(nHits > 2) {
      REQUIRE(pulses[2].getCellID() == 12347);
    }

}

TEST_CASE("Test the EvaluatorSvc pulse generation with a square pulse", "[SiliconPulseGeneration]") {
  eicrecon::SiliconPulseGeneration algo("SiliconPulseGeneration");
  eicrecon::SiliconPulseGenerationConfig cfg;

  // Square wave expression
  std::string expression = "charge * (time > [0] && time < [1]) ? 1 : 0";
  cfg.pulse_shape_function = expression;
  cfg.pulse_shape_params = {1.0 * edm4eic::unit::ns, 2.0 * edm4eic::unit::ns}; // Example parameters for the square pulse
  cfg.ignore_thres = 0.1;
  cfg.timestep = 0.1 * edm4eic::unit::ns;
  cfg.min_sampling_time = 0.1 * edm4eic::unit::ns;

  algo.applyConfig(cfg);
  algo.init();

  float charge = 10.0;
  float time   = 0.0;

  edm4hep::SimTrackerHitCollection hits_coll;
  hits_coll.create(12345, charge, time); // cellID, charge, time

  edm4hep::TimeSeriesCollection pulses = algo.GeneratePulses(hits_coll);

  REQUIRE(pulses.size() == 1);
  REQUIRE(pulses[0].getCellID() == 12345);
  REQUIRE(pulses[0].getTime() == 0.0);
  auto adcs = pulses[0].getAdcCounts();
  REQUIRE(adcs.size() == 10); // Two time bins for the square pulse
  for(auto adc:adcs){
    REQUIRE(adc == charge); // All time bins should be zero
  }
}
