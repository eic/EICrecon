// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Simon Gardner

#pragma once

namespace eicrecon {
  struct FarDetectorTrackerClusterConfig {

    std::string readout{""};
    std::string moduleField{"module"};
    std::string layerField{"layer"};
    std::string xField{"x"};
    std::string yField{"y"};

    int n_module{2};
    int n_layer{4};

  };
}
