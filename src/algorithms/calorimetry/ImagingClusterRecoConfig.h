// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Sylvester Joosten, Whitney Armstrong

#pragma once
#include <vector>
#include <edm4eic/unit_system.h>

namespace eicrecon {

struct ImagingClusterRecoConfig {

  int trackStopLayer = 9;

  // List of PDGs that are treated as promptly decaying
  std::vector<int> promptDecayPDGs{111, 221, 331, 310, 3122};

  double crossingWindow{0.0 * edm4eic::unit::ns};
  double crossingMargin{0.01 * edm4eic::unit::ns};
};

} // namespace eicrecon
