// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Minjung Kim

#pragma once

#include <vector>

namespace eicrecon {
struct AmbiguitySolverConfig {
  /// Maximum amount of shared hits per track.
  std::uint32_t maximum_shared_hits = 1;
  /// Maximum number of iterations
  std::uint32_t maximum_iterations = 100000;
  /// Minimum number of measurement to form a track.
  std::size_t n_measurements_min = 3;
};
} // namespace eicrecon
