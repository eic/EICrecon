// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Minjung Kim

#pragma once

#include <vector>

namespace eicrecon {
struct AmbiguitySolverConfig {
  /// Maximum amount of shared hits per track.
  std::uint32_t m_maximumSharedHits = 1;
  /// Maximum number of iterations
  std::uint32_t m_maximumIterations = 100000;
  /// Minimum number of measurement to form a track.
  std::size_t m_nMeasurementsMin = 3;
};
} // namespace eicrecon
