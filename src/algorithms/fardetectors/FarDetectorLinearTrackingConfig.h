// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Simon Gardner

#pragma once

#include <spdlog/spdlog.h>
#include "FarDetectorTrackerClusterConfig.h"

namespace eicrecon {
  struct FarDetectorLinearTrackingConfig {

    FarDetectorTrackerClusterConfig detconf;

    int   layer_hits_max{4};

    float chi2_max{4};

  };
}
