// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Simon Gardner

#pragma once

namespace eicrecon {

  struct SplitGeometryConfig {

    std::vector<std::vector<int>> divisions{};

    std::string readout{""};
    std::vector<std::string> division{};

  };

} // eicrecon
