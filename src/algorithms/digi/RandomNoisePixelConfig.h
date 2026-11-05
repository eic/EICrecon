// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minjung Kim, Joshua Sobaljic, Shujie Li

#pragma once

#include <edm4eic/unit_system.h>
#include <string>

namespace eicrecon {

struct RandomNoisePixelConfig {
  // Keep geometry initialization and event generation off unless explicitly enabled.
  bool addNoise = false;

  // Probability that one pixel fires from noise in one event. The same value is
  // applied to every SVT layer; the pixel pitch comes from DD4hep segmentation.
  double noise_rate_per_pixel_per_event = 2.0e-7;

  // DD4hep readout whose sensitive components and segmentation should be used.
  std::string readout_name = "VertexBarrelHits";

  // If > 0, noise hit times are drawn uniformly in [-timeWindow/2, +timeWindow/2];
  // otherwise all noise hits are at time 0.
  double timeWindow = 0 * edm4eic::unit::ns;
};

} // namespace eicrecon
