// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Tristan Protzman

#pragma once

namespace eicrecon {
    struct ProtoClusterMergerConfig {
        // How should clusters in different calorimeters be merged?
        // 0 - No merging
        // 1 - By cluster eta/phi
        uint merge_scheme = 0;

        float barrel_emcal_fraction = 0.5;
        float barrel_merge_distance = 0.1;
    };
} // namespace eicrecon