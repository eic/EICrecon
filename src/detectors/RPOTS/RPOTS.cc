// Copyright 2023, Alex Jentsch
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>

#include <extensions/jana/JChainFactoryGeneratorT.h>

#include <global/fardetectors/FarDetectorReconstruction_factory.h>
#include <algorithms/fardetectors/MatrixTransferStaticConfig.h>


extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    using namespace eicrecon;

    MatrixTransferStaticConfig recon_cfg;

    //Static transport matrix for OffMomentum detectors
    recon_cfg.aX.move = {{2.102403743, 29.11067626},
		    {0.186640381, 0.192604619}};
    recon_cfg.aY = {{0.0000159900, 3.94082098},
		    {0.0000079946, -0.1402995}};

    recon_cfg.local_x_offset       =  0.0;        //in mm --> this is from mis-alignment of the detector
    recon_cfg.local_y_offset       =  0.0;        //in mm --> this is from mis-alignment of the detector
    recon_cfg.local_x_slope_offset = -0.00622147; //in mrad
    recon_cfg.local_y_slope_offset = -0.0451035;  //in mrad
    recon_cfg.nomMomentum          =  275.0;      //in GEV --> exactly half of the top energy momentum (for proton spectators from deuteron breakup)

    app->Add(new JFactoryGeneratorT<FarDetectorReconstruction_factory>({"ForwardRomanPotHits"},"ForwardRomanPotRecParticles",recon_cfg));

}
}
