// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Aiden Wu

#include <catch2/catch_test_macros.hpp>
#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <cmath>
#include <cstddef>
#include <memory>
#include <tuple>

#include "algorithms/calorimetry/EdepToSiPMConversion.h"
#include "algorithms/calorimetry/EdepToSiPMConversionConfig.h"

TEST_CASE("Convert deposited energy to fired SiPM pixels", "[EdepToSiPMConversion]") {
  eicrecon::EdepToSiPMConversion algo("EdepToSiPMConversion");
  eicrecon::EdepToSiPMConversionConfig cfg;
  cfg.edep_to_npe                = 2;
  cfg.num_effective_sipm_pixels = 100;
  algo.applyConfig(cfg);
  algo.init();

  edm4hep::EventHeaderCollection headers;
  headers.create(1, 1);

  constexpr std::size_t num_trials = 20000;
  constexpr double edep             = 0.5;
  edm4hep::SimCalorimeterHitCollection hits;
  for (std::size_t i = 0; i < num_trials; i++) {
    hits.create().setEnergy(edep);
  }

  auto output_hits = std::make_unique<edm4hep::SimCalorimeterHitCollection>();
  algo.process(std::make_tuple(&headers, &hits), std::make_tuple(output_hits.get()));

  double sum         = 0;
  double squared_sum = 0;
  for (const auto& hit : *output_hits) {
    const double n_pixels_fired = hit.getEnergy();
    REQUIRE(n_pixels_fired >= 1);
    REQUIRE(n_pixels_fired <= cfg.num_effective_sipm_pixels);
    sum += n_pixels_fired;
    squared_sum += n_pixels_fired * n_pixels_fired;
  }

  const double mean_npe          = edep * cfg.edep_to_npe;
  const double sample_mean       = sum / num_trials;
  const double sample_variance   = squared_sum / num_trials - sample_mean * sample_mean;
  const double probability       = -std::expm1(-mean_npe / cfg.num_effective_sipm_pixels);
  const double expected_mean     = cfg.num_effective_sipm_pixels * probability;
  const double expected_variance = expected_mean * (1 - probability);

  REQUIRE(std::abs(sample_mean - expected_mean) < 0.05);
  REQUIRE(std::abs(sample_variance - expected_variance) < 0.1);
}
