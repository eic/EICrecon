// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

namespace eicrecon {

struct EnergyPositionClusterMergerConfig {

  double energyRelTolerance{0.5};
  double phiTolerance{0.1};
  double etaTolerance{0.2};
};

} // namespace eicrecon
