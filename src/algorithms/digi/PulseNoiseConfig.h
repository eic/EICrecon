// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Simon Gardner, Minho Kim

#pragma once

namespace eicrecon {

struct PulseNoiseConfig {

  // Number of poles in the noise filter
  std::size_t poles = 5;

  // Noise variance
  double variance = 1;

  // Noise alpha
  double alpha = 0.5;

  // Noise scale
  double scale = 1000;

  // Noise offset
  double pedestal = 0;
};

} // namespace eicrecon
