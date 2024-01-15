// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <string>

namespace eicrecon {

    struct CalorimeterClusterRecoCoGConfig {

        std::string energyWeight;

        double sampFrac = 1.;
        double logWeightBase = 3.6;

        //optional:  have the log weight base depend on the energy
        // logWeightBase+logWeightBase_lin*l+logWeightBase_quad*l*l
        // where l = log(cl.getEnergy()/logWeightBase_Eref)
        bool variableLogWeightBase=false;
        double logWeightBase_lin=0;
        double logWeightBase_quad=0;
        double logWeightBase_Eref=50;

        // Constrain the cluster position eta to be within
        // the eta of the contributing hits. This is useful to avoid edge effects
        // for endcaps.
        bool enableEtaBounds = false;

    };

} // eicrecon
