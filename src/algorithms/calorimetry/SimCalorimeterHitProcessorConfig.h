// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim, Sylvester Joosten, Derek Anderson

#pragma once

#include <string>
#include <vector>

namespace eicrecon {

struct SimCalorimeterHitProcessorConfig {

  // parameters for attenuation function
  std::vector<double> attPars;

  // fields for adding up energies and attenuate them
  std::string readout{""};
  std::string attenuationField{""};
  std::string mergeField{""};
};

} // namespace eicrecon
