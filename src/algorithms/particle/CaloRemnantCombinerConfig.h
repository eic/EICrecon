// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Subhadip Pal

#pragma once

#include <Evaluator/DD4hepUnits.h>

namespace eicrecon {

struct CaloRemnantCombinerConfig {

  ///! radius around seed to add EMCal clusters
  double deltaRAddEM = 0.03;

  ///! radius around see to add HCal clusters
  double deltaRAddH  = 0.15;

};

} // namespace eicrecon
