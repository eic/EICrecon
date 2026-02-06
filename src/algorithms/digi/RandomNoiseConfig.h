// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 EIC-FT

#pragma once

#include <vector>
#include <utility>
#include <string>

namespace eicrecon {

struct RandomNoiseConfig {
  // Enable or disable noise injection.
  bool addNoise = false;

  // Average number of noise hits to inject per system.
  // This will be used as the mean for a Poisson distribution.
  int n_noise_hits_per_system = 100;

  std::string readout_name = "VertexBarrelHits";
  std::vector<int> layer_id;
  std::vector<int> n_noise_hits_per_layer;
  std::vector<std::string> detector_names;
  // Standalone mode: ignore input and produce a separate noise-only collection.
  // CollectionCollector can merge later. Default: true.
  std::string layer_field_hint;
};

} // namespace eicrecon
