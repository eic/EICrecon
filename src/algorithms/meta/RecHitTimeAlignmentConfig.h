// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2024 Whitney Armstrong, Wouter Deconinck, Dmitry Romanov

#pragma once

namespace eicrecon {

struct RecHitTimeAlignmentConfig {
  double reference_inverse_velocity =
      0.0034; // ns/mm estimated by MC average time of flight / distance from IP to calorimeter
};

} // namespace eicrecon
