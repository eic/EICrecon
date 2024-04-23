// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Simon Gardner

#pragma once

#include <edm4eic/unit_system.h>

namespace eicrecon {
  struct FarDetectorTrackerClusterConfig {

    // Readout identifiers for dividing detector
    std::string readout{""};
    std::string xField{"x"};
    std::string yField{"y"};

    // Timing limit to add a hit to a cluster
    double time_limit{10*edm4eic::unit::ns};

  };
}
