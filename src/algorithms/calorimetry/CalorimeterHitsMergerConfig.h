// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Wouter Deconinck

#pragma once

#include <string>
#include <vector>

namespace eicrecon {

  struct CalorimeterHitsMergerConfig {

    std::string              readout{""};
    std::vector<std::string> fields{};
    std::vector<int>         refs{};

  };

} // eicrecon
