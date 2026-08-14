// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Takuya Kumaoka

#pragma once

namespace eicrecon {

struct HitTimeAlignmentConfig {
  double refInverseVelocity =
      0.0034; //< ns/mm estimated by MC average time of flight / distance from IP to calorimeter
};

} // namespace eicrecon
