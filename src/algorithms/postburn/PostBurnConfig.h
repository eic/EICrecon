// Copyright 2024, Alex Jentsch, Jihee Kim, Brian Page
// Subject to the terms in the LICENSE file found in the top-level directory.
//
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

