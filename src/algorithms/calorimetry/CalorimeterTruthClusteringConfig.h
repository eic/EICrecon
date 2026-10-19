// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ePIC Collaboration

#pragma once

#include <vector>

namespace eicrecon {

struct CalorimeterTruthClusteringConfig {
  // List of PDGs that are treated as promptly decaying
  std::vector<int> promptDecayPDGs{111, 221, 331, 310, 3122};
};

} // namespace eicrecon
