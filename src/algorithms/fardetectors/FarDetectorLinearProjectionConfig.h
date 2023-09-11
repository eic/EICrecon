// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Simon Gardner

#pragma once

namespace eicrecon {
  struct FarDetectorLinearProjectionConfig {

    float plane_position[3]{0.0, 0.0, 0.0};
    float plane_tangent[3] {1.0, 0.0, 0.0};

  };
}
