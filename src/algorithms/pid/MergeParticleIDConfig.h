// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Christopher Dilks

#pragma once

#include <algorithms/logger.h>
#include <spdlog/spdlog.h>

namespace eicrecon {

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

  // stream all parameters
  friend std::ostream& operator<<(std::ostream& os, const MergeParticleIDConfig& cfg) {
    // print all parameters
    os << fmt::format("{:=^60}", " MergeParticleIDConfig Settings ") << std::endl;
    auto print_param = [&os](auto name, auto val) {
      os << fmt::format("  {:>20} = {:<}", name, val) << std::endl;
    };
    print_param("mergeMode", cfg.mergeMode);
    os << fmt::format("{:=^60}", "") << std::endl;
    return os;
  }
};

} // namespace eicrecon
