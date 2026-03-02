// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tomas Sosa, Wouter Deconinck

#pragma once
#include <string>

namespace eicrecon {

struct CalorimeterEOverPCutConfig {
  double eOverPCut       = 0.74;
  int maxLayer           = 12;
  std::string readout    = "";
  std::string layerField = "layer";
};

} // namespace eicrecon
