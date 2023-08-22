// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Simon Gardner

#pragma once

#include <spdlog/spdlog.h>

namespace eicrecon {
  struct FarTrackerClusterConfig {
    
    int   n_module{2};
    int   n_layer{4};

    int   layer_hits_max{4};
    
    float chi2_max{4};
      
  };
}
