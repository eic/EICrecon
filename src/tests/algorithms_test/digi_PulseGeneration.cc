// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025, Dmitry Kalinkin, Simon Gardner

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/SimTrackerHitCollection.h>
#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 1)
#include <edm4eic/SimPulseCollection.h>
#else
#include <edm4hep/TimeSeriesCollection.h>
#endif
#include <podio/RelationRange.h>
#include <cmath>
#include <cstddef>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "algorithms/digi/PulseGeneration.h"
#include "algorithms/digi/PulseGenerationConfig.h"

#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 1)
using PulseType = edm4eic::SimPulse;
#else
using PulseType = edm4hep::TimeSeries;
#endif

TEST_CASE("PulseGeneration generates correct number of pulses", "[PulseGeneration]") {

  eicrecon::PulseGeneration<edm4hep::SimTrackerHit> algo("PulseGeneration");
  eicrecon::PulseGenerationConfig cfg;
  cfg.pulse_shape_function = "LandauPulse"; // Example pulse shape
  cfg.pulse_shape_params   = {1.0, 1.0};    // Example parameters for the pulse shape
  cfg.ignore_thres         = 1;

  algo.applyConfig(cfg);
  algo.init();

  SECTION("from different hits") {
    std::size_t nHits = GENERATE(1, 2, 3);

    edm4hep::SimTrackerHitCollection hits_coll;

    for (std::size_t i = 0; i < nHits; i++) {
      hits_coll.create(12345 + i, 10.0, 5.0); // cellID, charge, time
    }

    auto pulses = std::make_unique<PulseType::collection_type>();

    auto input  = std::make_tuple(&hits_coll);
    auto output = std::make_tuple(pulses.get());

    algo.process(input, output);

    REQUIRE(pulses->size() == nHits);
    REQUIRE((*pulses)[0].getCellID() == 12345);
    if (nHits > 1) {
      REQUIRE((*pulses)[1].getCellID() == 12346);
    }
    if (nHits > 2) {
      REQUIRE((*pulses)[2].getCellID() == 12347);
    }
  }
}

TEST_CASE("Test the EvaluatorSvc pulse generation with a square pulse", "[PulseGeneration]") {

  eicrecon::PulseGeneration<edm4hep::SimTrackerHit> algo("PulseGeneration");
  eicrecon::PulseGenerationConfig cfg;

  // Square wave expression
  std::string expression = "(time >= param0 && time < param1) ? charge : 0";

  double startTime      = 0.0 * edm4eic::unit::ns;
  double endTime        = 1.0 * edm4eic::unit::ns;
  std::size_t nTimeBins = 10;
  double timeStep       = (endTime - startTime) / nTimeBins;

  cfg.pulse_shape_function = expression;
  cfg.pulse_shape_params   = {startTime, endTime}; // Example parameters for the square pulse
  cfg.ignore_thres         = 1;
  cfg.timestep             = timeStep;
  cfg.min_sampling_time    = startTime + timeStep;

  algo.applyConfig(cfg);
  algo.init();

  double charge      = 10.0 * cfg.ignore_thres;
  double time        = GENERATE_COPY(0.0, 0.5 * timeStep, timeStep);
  float rounded_time = std::floor(time / timeStep) * timeStep;

  edm4hep::SimTrackerHitCollection hits_coll;
  hits_coll.create(12345, charge, time); // cellID, charge, time

  auto pulses = std::make_unique<PulseType::collection_type>();

  auto input  = std::make_tuple(&hits_coll);
  auto output = std::make_tuple(pulses.get());

  algo.process(input, output);

  REQUIRE(pulses->size() == 1);
  REQUIRE((*pulses)[0].getCellID() == 12345);
  REQUIRE((*pulses)[0].getTime() == rounded_time);
  auto amplitudes = (*pulses)[0].getAmplitude();
  REQUIRE(amplitudes.size() == nTimeBins); // Two time bins for the square pulse
  for (auto amplitude : amplitudes) {
    REQUIRE(amplitude == charge); // All time bins should be zero
  }
}
