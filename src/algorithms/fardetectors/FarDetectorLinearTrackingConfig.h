// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Simon Gardner

#pragma once

namespace eicrecon {
  struct FarDetectorLinearTrackingConfig {

    std::string readout{""};
    std::string moduleField{"module"};
    std::string layerField{"layer"};

    int   layer_hits_max{4};

    float chi2_max{4};

  };
}
