// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Simon Gardner

#pragma once

namespace eicrecon {
  struct FarDetectorLinearTrackingConfig {

    int   layer_hits_max{10};
    float chi2_max{0.001};
    int   n_layer{4};

  };
}
