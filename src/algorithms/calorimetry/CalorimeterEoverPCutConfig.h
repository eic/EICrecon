// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tomas Sosa, Wouter Deconinck

#pragma once

namespace eicrecon {

struct CalorimeterEoverPCutConfig {
  double eOverPCut = 0.74;
  int maxLayer     = 12;
};

} // namespace eicrecon
