// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim, Sylvester Joosten, Derek Anderson, Wouter Deconinck

#pragma once

#include <string>
#include <vector>

namespace eicrecon {

struct SimCalorimeterHitProcessorConfig {

  // parameters for attenuation function
  std::vector<double> attPars;

  std::string readout{""};
  std::string attenuationReferencePositionName{""};
  // fields for merging hits
  std::vector<std::string> hitMergeFields{};
  // fields for merging contributions
  std::vector<std::string> contributionMergeFields{};
};

} // namespace eicrecon
