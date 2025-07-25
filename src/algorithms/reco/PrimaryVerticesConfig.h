// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Xin Dong

#pragma once

#include <string>
#include <DD4hep/DD4hepUnits.h>

namespace eicrecon {

struct PrimaryVerticesConfig {

  // For now these are wide open
  // In the future the cut should depend
  // on the generator settings
  float maxVr   = 50.0;    // mm
  float maxVz   = 500.0;   // mm
  float maxChi2 = 10000.0; //
  int minNtrk   = 1;       // >=
  int maxNtrk   = 1000000; // <=
};

} // namespace eicrecon
