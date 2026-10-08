// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 ePIC Collaboration

#pragma once

#include <vector>

namespace eicrecon {

/// Configuration for MeasurementToMille: selects tracks suitable for
/// silicon tracker alignment and controls which layers are constrained.
struct MeasurementToMilleConfig {
  /// Maximum chi2/NDF for a track to be used in alignment
  float maxChi2PerNDF = 5.0f;
  /// Minimum track momentum [GeV/c]
  float minMomentum = 1.0f;
  /// 0-based layer indices to fix (skip) in Millepede, e.g. {0} for a reference layer
  std::vector<int> fixedLayers;
};

} // namespace eicrecon
