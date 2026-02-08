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
#include <initializer_list>
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

TEST_CASE("Test early exit for Landau pulse never reaching threshold",
          "[PulseGeneration][EarlyExit]") {

  eicrecon::PulseGeneration<edm4hep::SimTrackerHit> algo("PulseGeneration");
  eicrecon::PulseGenerationConfig cfg;

  // Configure Landau pulse with parameters
  cfg.pulse_shape_function = "LandauPulse";
  cfg.pulse_shape_params   = {1.0, 1.0}; // gain=1.0, sigma_analog=1.0
  cfg.ignore_thres         = 10.0;       // Set threshold high enough that pulse won't reach it
  cfg.timestep             = 0.1 * edm4eic::unit::ns;
  cfg.min_sampling_time    = 0.0 * edm4eic::unit::ns;
  cfg.max_time_bins        = 1000;

  algo.applyConfig(cfg);
  algo.init();

  // Use low charge so the pulse never reaches ignore_thres
  // With gain=1.0, sigma=1.0, and Landau normalized, peak amplitude ~0.18 * charge
  // So charge=20 gives peak ~3.6, well below threshold of 10.0
  double charge = 20.0;
  double time   = 5.0 * edm4eic::unit::ns;

  edm4hep::SimTrackerHitCollection hits_coll;
  hits_coll.create(12345, charge, time);

  auto pulses = std::make_unique<PulseType::collection_type>();

  auto input  = std::make_tuple(&hits_coll);
  auto output = std::make_tuple(pulses.get());

  algo.process(input, output);

  // Pulse should not be generated since it never crosses threshold
  REQUIRE(pulses->size() == 0);
}

TEST_CASE("Test Landau pulse crossing threshold is not prematurely terminated",
          "[PulseGeneration][EarlyExit]") {

  eicrecon::PulseGeneration<edm4hep::SimTrackerHit> algo("PulseGeneration");
  eicrecon::PulseGenerationConfig cfg;

  // Configure Landau pulse
  cfg.pulse_shape_function = "LandauPulse";
  cfg.pulse_shape_params   = {1.0, 1.0}; // gain=1.0, sigma_analog=1.0
  cfg.ignore_thres         = 1.0;        // Set threshold low so pulse will cross it
  cfg.timestep             = 0.1 * edm4eic::unit::ns;
  cfg.min_sampling_time    = 2.0 * edm4eic::unit::ns; // Minimum sampling time
  cfg.max_time_bins        = 1000;

  algo.applyConfig(cfg);
  algo.init();

  // Use high enough charge that the pulse crosses threshold
  // With gain=1.0, sigma=1.0, peak amplitude ~0.18 * charge
  // So charge=10 gives peak ~1.8, above threshold of 1.0
  double charge = 10.0;
  double time   = 5.0 * edm4eic::unit::ns;

  edm4hep::SimTrackerHitCollection hits_coll;
  hits_coll.create(12345, charge, time);

  auto pulses = std::make_unique<PulseType::collection_type>();

  auto input  = std::make_tuple(&hits_coll);
  auto output = std::make_tuple(pulses.get());

  algo.process(input, output);

  // Pulse should be generated since it crosses threshold
  REQUIRE(pulses->size() == 1);
  REQUIRE((*pulses)[0].getCellID() == 12345);

  auto amplitudes = (*pulses)[0].getAmplitude();

  // Should have non-zero amplitude samples
  REQUIRE(amplitudes.size() > 0);

  // Check that the pulse was not prematurely terminated
  // It should sample at least until min_sampling_time after crossing threshold
  // Derive expected minimum samples from cfg to avoid hard-coding
  const auto expected_min_samples =
      static_cast<std::size_t>(std::ceil(cfg.min_sampling_time / cfg.timestep));
  REQUIRE(amplitudes.size() >= expected_min_samples);

  // Verify that some amplitudes are above threshold
  bool has_above_threshold = false;
  for (auto amplitude : amplitudes) {
    if (std::abs(amplitude) >= cfg.ignore_thres) {
      has_above_threshold = true;
      break;
    }
  }
  REQUIRE(has_above_threshold);
}

TEST_CASE("Test multi-modal expression pulse with early sub-threshold peak and later "
          "above-threshold peak",
          "[PulseGeneration][MultiModal]") {

  eicrecon::PulseGeneration<edm4hep::SimTrackerHit> algo("PulseGeneration");
  eicrecon::PulseGenerationConfig cfg;

  // Regression test for multi-modal pulses with EvaluatorSvc expressions.
  // Expression with two separated peaks:
  // - First peak near t=0 with amplitude 0.5*charge (below threshold)
  // - Second peak near t=5ns with amplitude 1.5*charge (above threshold)
  //
  // This verifies the algorithm does NOT prematurely exit after encountering
  // the first sub-threshold peak. For non-unimodal pulses (like arbitrary
  // EvaluatorSvc expressions), the algorithm must continue searching for
  // potential later peaks that may cross the threshold, rather than assuming
  // the pulse is "falling" and breaking early.
  std::string expression = "(time >= 0.0 && time <= 0.5 ? 0.5 * charge : 0.0) + "
                           "(time >= 5.0 && time <= 5.5 ? 1.5 * charge : 0.0)";

  cfg.pulse_shape_function = expression;
  cfg.pulse_shape_params   = {}; // No parameters needed for piecewise expression
  cfg.ignore_thres         = 1.0;
  cfg.timestep             = 0.1 * edm4eic::unit::ns;
  cfg.min_sampling_time =
      1.0 * edm4eic::unit::ns; // Min duration to continue sampling from the hit time
  cfg.max_time_bins = 200;     // Enough to capture both peaks

  algo.applyConfig(cfg);
  algo.init();

  // Use charge=1.0 so:
  // - First peak amplitude  = 0.5 * 1.0 = 0.5 (below threshold of 1.0)
  // - Second peak amplitude = 1.5 * 1.0 = 1.5 (above threshold of 1.0)
  // Peaks are at t=[0.0, 0.5] and t=[5.0, 5.5] respectively
  double charge = 1.0;
  double time   = 0.0 * edm4eic::unit::ns;

  edm4hep::SimTrackerHitCollection hits_coll;
  hits_coll.create(12345, charge, time);

  auto pulses = std::make_unique<PulseType::collection_type>();

  auto input  = std::make_tuple(&hits_coll);
  auto output = std::make_tuple(pulses.get());

  algo.process(input, output);

  // Pulse should be generated because the second peak crosses threshold
  REQUIRE(pulses->size() == 1);
  REQUIRE((*pulses)[0].getCellID() == 12345);

  auto amplitudes = (*pulses)[0].getAmplitude();

  // Should have sampled enough to capture the second peak
  REQUIRE(amplitudes.size() > 0);

  // Find the maximum amplitude - it should be near the second peak
  float max_amplitude = 0.0;
  std::size_t max_idx = 0;
  for (std::size_t i = 0; i < amplitudes.size(); i++) {
    if (std::abs(amplitudes[i]) > std::abs(max_amplitude)) {
      max_amplitude = amplitudes[i];
      max_idx       = i; // Valid since i < amplitudes.size() during iteration
    }
  }

  // The maximum should be above threshold (from the second peak)
  REQUIRE(std::abs(max_amplitude) > cfg.ignore_thres);

  // The maximum should occur in the second peak region (t=[5.0, 5.5] ns)
  // Account for the pulse start time when calculating the time of maximum
  // Note: pulse start time is aligned to the timestep grid, and the first sample
  // in the amplitudes array is at pulse_start_time + timestep
  double pulse_start_time = (*pulses)[0].getTime();
  double max_time         = pulse_start_time + (max_idx + 1) * cfg.timestep;
  REQUIRE(max_time >= 5.0 * edm4eic::unit::ns);
  REQUIRE(max_time <= 5.6 * edm4eic::unit::ns); // Allow for one timestep beyond peak end

  // Verify we didn't exit early - should have samples beyond the actual peak
  // Compare against the computed max_time to ensure at least one additional timestep
  // Note: the time of amplitudes[idx] is pulse_start_time + (idx + 1) * timestep
  double last_sampled_time = pulse_start_time + amplitudes.size() * cfg.timestep;
  REQUIRE(last_sampled_time >= max_time + cfg.timestep);
}
