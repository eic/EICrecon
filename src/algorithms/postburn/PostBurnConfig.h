// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Alex Jentsch, Jihee Kim, Brian Page
//

#pragma once

namespace eicrecon {

    struct PostBurnConfig {

        bool      pidAssumePionMass = false;
        double    crossingAngle     = 0.025;
        double    pidPurity         = 0.51;
        bool      correctBeamFX     = true;
        bool      pidUseMCTruth     = true;

    };

}
