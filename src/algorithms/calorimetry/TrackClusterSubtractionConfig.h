// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Derek Anderson

#pragma once

#include <string>

namespace eicrecon {

  struct TrackClusterSubtractionConfig {

    std::string idCalo = "HcalBarrel_ID";  // id of calorimeter to match projections to

    /* TODO parameters will go here */

    // scale for hit-track distance
    double transverseEnergyProfileScale = 1.0;

  };  // end TrackClusterSubtractionConfig

}  // end eicrecon namespace
