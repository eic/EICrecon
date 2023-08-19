// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <string>

namespace eicrecon {

    struct CalorimeterClusterRecoCoGConfig {

        std::string energyWeight;
        std::string moduleDimZName;

        double sampFrac = 1.;
        double logWeightBase = 3.6;
        double depthCorrection = 0.;

        // Constrain the cluster position eta to be within
        // the eta of the contributing hits. This is useful to avoid edge effects
        // for endcaps.
        bool enableEtaBounds = false;

    };

} // eicrecon
