// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Simon Gardner

#pragma once

namespace eicrecon {
struct FarDetectorLinearProjectionConfig {

  std::vector<float> plane_position = {0.0, 0.0, 0.0};
  std::vector<float> plane_a        = {0.0, 1.0, 0.0};
  std::vector<float> plane_b        = {0.0, 0.0, 1.0};
};
} // namespace eicrecon
