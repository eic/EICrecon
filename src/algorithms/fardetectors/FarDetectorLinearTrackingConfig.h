// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Simon Gardner

#pragma once

namespace eicrecon {
struct FarDetectorLinearTrackingConfig {

  std::size_t layer_hits_max{10};
  float chi2_max{0.001};
  std::size_t n_layer{4};

  // Restrict hit direction
  bool restrict_direction{true};
  float optimum_theta{0.026};
  float optimum_phi{0};
  float step_angle_tolerance{0.05};
};
} // namespace eicrecon
