// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023, Christopher Dilks, Luigi Dello Stritto

#pragma once

#include <spdlog/spdlog.h>

namespace eicrecon {
  struct FarDetectorTrackerClusterConfig {

    std::string readout{""};
    std::string moduleField{"module"};
    std::string layerField{"layer"};
    std::string xField{"x"};
    std::string yField{"y"};

    int module_idx{0};
    int layer_idx{0};
    int x_idx{0};
    int y_idx{0};

    int n_module{2};
    int n_layer{4};

  };
}
