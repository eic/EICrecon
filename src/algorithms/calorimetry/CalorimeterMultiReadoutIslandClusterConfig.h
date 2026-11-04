// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Aiden Wu

#pragma once

#include <cstdint>
#include <vector>

#include "CalorimeterIslandClusterConfig.h"

namespace eicrecon {

struct CalorimeterMultiReadoutIslandClusterReadoutConfig {
  std::vector<std::uint32_t> collectionIDs;
  CalorimeterIslandClusterConfig island{};
};

struct CalorimeterMultiReadoutIslandClusterConfig {

  // Readout configurations and source collections.
  std::vector<CalorimeterMultiReadoutIslandClusterReadoutConfig> readouts;

  // Global neighbor distance in EDM units (mm); zero disables it.
  // Used between readouts and within readouts without an adjacency rule.
  double globalDist{0.};
};

} // namespace eicrecon
