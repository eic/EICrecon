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
    recon_cfg.aX = {{1.6248, 12.966293},
		    {0.1832, -2.8636535}};
    recon_cfg.aY = {{0.0001674, -28.6003},
		    {0.0000837, -2.87985}};

    recon_cfg.local_x_offset       = -11.9872;  // in mm --> this is from mis-alignment of the detector
    recon_cfg.local_y_offset       = -0.0146;   //in mm --> this is from mis-alignment of the detector
    recon_cfg.local_x_slope_offset = -14.75315; //in mrad
    recon_cfg.local_y_slope_offset = -0.0073;   //in mrad
    recon_cfg.nomMomentum          =  137.5;    //in GEV --> exactly half of the top energy momentum (for proton spectators from deuteron breakup)

    app->Add(new JFactoryGeneratorT<FarDetectorReconstruction_factory>({"ForwardOffMTrackerHits"},"ForwardOffMRecParticles",recon_cfg));

}
}
