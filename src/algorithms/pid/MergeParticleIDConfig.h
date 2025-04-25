// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Christopher Dilks

#pragma once

#include <algorithms/logger.h>
#include <spdlog/spdlog.h>

namespace eicrecon {

class MergeParticleID;

class MergeParticleIDConfig {
public:
  /////////////////////////////////////////////////////
  // CONFIGURATION PARAMETERS
  //   NOTE: some defaults are hard-coded here; override externally

  // decide how to combine the weights
  enum math_enum { kAddWeights, kMultiplyWeights };
  int mergeMode = kAddWeights;

  //
  /////////////////////////////////////////////////////

  // print all parameters
  template <algorithms::LogLevel lvl = algorithms::LogLevel::kDebug>
  constexpr void Print(const MergeParticleID* logger) const;
};
} // namespace eicrecon
