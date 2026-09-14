// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Subhadip Pal

#pragma once

namespace eicrecon {

struct CaloRemnantCombinerConfig {

  ///! DeltaR around seed to add Ecal clusters
  double ecalDeltaR = 0.03;

  ///! DeltaR around seed to add Hcal clusters
  double hcalDeltaR = 0.15;
};

} // namespace eicrecon
