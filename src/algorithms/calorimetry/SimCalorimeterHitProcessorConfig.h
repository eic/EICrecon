// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim, Sylvester Joosten, Derek Anderson, Wouter Deconinck

#pragma once

#include <string>
#include <vector>

namespace eicrecon {

struct SimCalorimeterHitProcessorConfig {

  // parameters for attenuation function
  // [0] * exp(-|z_ref - z| / [1]) + (1 - [0]) * exp(-|z_ref - z| / [2])
  // specified in edm4eic::units where dimensionfull
  std::vector<double> attenuationParameters;

  std::string readout{""};
  std::string attenuationReferencePositionName{""};
  // fields for merging hits
  std::vector<std::string> hitMergeFields{};
  // fields for merging contributions
  std::vector<std::string> contributionMergeFields{};

  // inverse of the propagation speed of hits in the detector material
  // declared as an inverse to avoid division by zero
  double inversePropagationSpeed{};
  // detector-related time delay (e.g., scintillation)
  double fixedTimeDelay{};
};

} // namespace eicrecon
