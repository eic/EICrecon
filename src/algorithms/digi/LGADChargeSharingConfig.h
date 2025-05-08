// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul

#pragma once

#include <DD4hep/DD4hepUnits.h>

namespace eicrecon {

struct LGADChargeSharingConfig {
  // Parameters of AC-LGAD signal generation
  double sigma_sharingx;
  double sigma_sharingy;

  std::string readout;
  std::string same_sensor_condition;
  std::vector<std::string> neighbor_fields;
};

} // namespace eicrecon
