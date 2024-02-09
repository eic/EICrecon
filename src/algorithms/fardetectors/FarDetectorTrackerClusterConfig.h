// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Simon Gardner

#pragma once

#include <DD4hep/DD4hepUnits.h>

namespace eicrecon {
  struct FarDetectorTrackerClusterConfig {

    // Readout identifiers for dividing detector
    std::string readout{""};
    std::string xField{"x"};
    std::string yField{"y"};

    // Timing limit to add a hit to a cluster
    double time_limit{10*dd4hep::ns};

  };
}
