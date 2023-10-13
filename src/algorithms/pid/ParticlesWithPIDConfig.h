// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

namespace eicrecon {

    struct ParticlesWithPIDConfig {
        double momentumRelativeTolerance = 100.0; /// Matching momentum effectively disabled

        double phiTolerance = 0.1; /// Matching phi tolerance [rad]

        double etaTolerance = 0.2; /// Matching eta tolerance
    };

} // eicrecon
