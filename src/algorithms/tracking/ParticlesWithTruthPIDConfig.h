// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

namespace eicrecon {

    struct ParticlesWithTruthPIDConfig {
        double momentumRelativeTolerance = 0.1;    /// Matching momentum tolerance requires 10% by default;

        double phiTolerance = 0.030;        /// Matching phi tolerance [mrad]

        double etaTolerance = 0.2;          /// Matching eta tolerance of 0.2
    };

} // eicrecon

