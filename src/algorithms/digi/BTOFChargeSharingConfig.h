// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul

#pragma once

#include <DD4hep/DD4hepUnits.h>

namespace eicrecon {

struct BTOFChargeSharingConfig {
  // Parameters of AC-LGAD signal generation
  double sigma_sharingx = 0.1 * dd4hep::cm;
  double sigma_sharingy = 0.5 * dd4hep::cm;

  std::string readout = "TOFBarrelHits";
};

} // namespace eicrecon
