// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023, Christopher Dilks, Luigi Dello Stritto

#pragma once

#include <spdlog/spdlog.h>

namespace eicrecon {
  struct FarDetectorMLReconstructionConfig {

    int n_module{2};
    int n_layer{4};

  };
}
