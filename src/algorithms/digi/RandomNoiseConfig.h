// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 EIC-FT

#pragma once

#include <vector>
#include <utility>

namespace eicrecon {

// Enum to specify subdetector regions for targeted noise injection.
enum class SubdetectorRegion { barrel, backward, forward };

struct RandomNoiseConfig {
  // Enable or disable noise injection.
  bool addNoise = false;

  // Average number of noise hits to inject per system.
  // This will be used as the mean for a Poisson distribution.
  int n_noise_hits_per_system = 100;

  std::string readout_name="VertexBarrelHits";
};

} // namespace eicrecon