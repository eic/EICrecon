// Copyright 2023, Alex Jentsch
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <vector>

#include "algorithms/fardetectors/MatrixTransferStaticConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/fardetectors/MatrixTransferStatic_factory.h"


extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    using namespace eicrecon;

    MatrixTransferStaticConfig recon_cfg;

    //Static transport matrix for Roman Pots detectors
    recon_cfg.aX = {{2.102403743, 29.11067626},
                    {0.186640381, 0.192604619}};
    recon_cfg.aY = {{0.0000159900, 3.94082098},
                    {0.0000079946, -0.1402995}};

    recon_cfg.local_x_offset       =  0.0;        // in mm --> this is from misalignment of the detector
    recon_cfg.local_y_offset       =  0.0;        // in mm --> this is from misalignment of the detector
    recon_cfg.local_x_slope_offset = -0.00622147; // in mrad
    recon_cfg.local_y_slope_offset = -0.0451035;  // in mrad
    recon_cfg.nomMomentum          =  275.0;      // in GEV --> exactly half of the top energy momentum (for proton spectators from deuteron breakup)

    recon_cfg.hit1minZ = 25099.0;
    recon_cfg.hit1maxZ = 26022.0;
    recon_cfg.hit2minZ = 27099.0;
    recon_cfg.hit2maxZ = 28022.0;

    recon_cfg.readout              = "ForwardRomanPotHits";

    app->Add(new JOmniFactoryGeneratorT<MatrixTransferStatic_factory>("ForwardRomanPotRecParticles",{"ForwardRomanPotHits"},{"ForwardRomanPotRecParticles"},recon_cfg,app));

}
}
