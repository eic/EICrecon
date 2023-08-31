// Copyright 2023, Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Acts/Propagator/Navigator.hpp>

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include "extensions/jana/JChainMultifactoryGeneratorT.h"


#include <factories/digi/SiliconTrackerDigi_factoryT.h>
#include <factories/fardetectors/FarDetectorTrackerCluster_factoryT.h>
#include <factories/fardetectors/FarDetectorLinearTracking_factoryT.h>
#include <factories/fardetectors/FarDetectorMLReconstruction_factoryT.h>


extern "C" {
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    // -------------------------------
    // TODO: Separate algorithms from factories and add config parameters
    // -------------------------------

    // Digitization
    SiliconTrackerDigiConfig digi_cfg;
    //digi_cfg.timeResolution = 2.5; // Change timing resolution.

    //Clustering config
    FarDetectorTrackerClusterConfig cluster_cfg;
    //Ensure same detector is passed to digi and clustering
    cluster_cfg.readout = "TaggerTrackerHits";

    FarDetectorLinearTrackingConfig tracking_cfg;
    tracking_cfg.detconf = cluster_cfg;

    FarDetectorMLReconstructionConfig recon_cfg;

    app->Add(new JChainMultifactoryGeneratorT<SiliconTrackerDigi_factoryT>(
	 "TaggerTrackerRawHit",
         {cluster_cfg.readout},
         {"TaggerTrackerRawHit"},
         {
	   .threshold = 1 * dd4hep::keV,
	   .timeResolution = 10,
	 },
         app
    ));


    // Clustering of hits
    app->Add(new JChainMultifactoryGeneratorT<FarDetectorTrackerCluster_factoryT>("TaggerTrackerClusterPositions",{"TaggerTrackerRawHit"},{"TaggerTrackerClusterPositions"}, cluster_cfg, app));

    // Very basic reconstrution of a single track
    app->Add(new JChainMultifactoryGeneratorT<FarDetectorLinearTracking_factoryT>("LowQ2Tracks",{"TaggerTrackerClusterPositions"},{"LowQ2Tracks"}, tracking_cfg, app));

    // Initial particle reconstruction
    app->Add(new JChainMultifactoryGeneratorT<FarDetectorMLReconstruction_factoryT>("LowQ2Trajectories",{"LowQ2Tracks"},{"LowQ2Trajectories"}, recon_cfg, app));

  }
}
