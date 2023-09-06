// Copyright 2023, Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include "extensions/jana/JChainMultifactoryGeneratorT.h"

#include "factories/digi/SiliconTrackerDigi_factoryT.h"
#include "factories/fardetectors/FarDetectorTrackerCluster_factoryT.h"
#include "factories/fardetectors/FarDetectorLinearTracking_factoryT.h"
#include "factories/fardetectors/FarDetectorMLReconstruction_factoryT.h"


extern "C" {
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    //Clustering config
    FarDetectorTrackerClusterConfig cluster_cfg;
    //Ensure same detector is passed to digi and clustering
    cluster_cfg.readout = "TaggerTrackerHits";

    FarDetectorLinearTrackingConfig tracking_cfg;
    tracking_cfg.readout = cluster_cfg.readout;

    FarDetectorMLReconstructionConfig recon_cfg;

    // Digitization of silicon hits
    app->Add(new JChainMultifactoryGeneratorT<SiliconTrackerDigi_factoryT>(
         "TaggerTrackerRawHits",
         {"TaggerTrackerHits"},
         {"TaggerTrackerRawHits"},
         {
           .threshold = 1 * dd4hep::keV,
           .timeResolution = 2 * dd4hep::ns,
         },
         app
    ));

    // Clustering of hits
    app->Add(new JChainMultifactoryGeneratorT<FarDetectorTrackerCluster_factoryT>(
        "TaggerTrackerClusterPositions",
        {"TaggerTrackerRawHits"},
        {"TaggerTrackerClusterPositions"},
        {
          .readout = "TaggerTrackerHits"
        },
        app
    ));

    // Reconstrution of tracks on common plane
    app->Add(new JChainMultifactoryGeneratorT<FarDetectorLinearTracking_factoryT>("LowQ2Tracks",{"TaggerTrackerClusterPositions"},{"LowQ2Tracks"}, tracking_cfg, app));

    // Vector reconstruction at origin
    app->Add(new JChainMultifactoryGeneratorT<FarDetectorMLReconstruction_factoryT>("LowQ2Trajectories",{"LowQ2Tracks"},{"LowQ2Trajectories","LowQ2TrackParameters"}, recon_cfg, app));

  }
}
