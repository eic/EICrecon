// Copyright 2023, Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <string>

#include "factories/digi/SiliconTrackerDigi_factoryT.h"
#include "factories/fardetectors/FarDetectorTrackerCluster_factoryT.h"
#include "factories/fardetectors/FarDetectorLinearTracking_factoryT.h"
#include "factories/fardetectors/FarDetectorLinearProjection_factoryT.h"
#include "factories/fardetectors/FarDetectorMLReconstruction_factoryT.h"

#include "algorithms/interfaces/WithPodConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/digi/SiliconTrackerDigi_factory.h"


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

    FarDetectorLinearProjectionConfig projection_cfg;

    FarDetectorMLReconstructionConfig recon_cfg;

    // Digitization of silicon hits
    app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
         "TaggerTrackerRawHits",
         {"TaggerTrackerHits"},
         {"TaggerTrackerRawHits"},
         {
           .threshold = 1.5 * dd4hep::keV,
           .timeResolution = 2 * dd4hep::ns,
         },
         app
    ));

    // Clustering of hits
    app->Add(new JOmniFactoryGeneratorT<FarDetectorTrackerCluster_factoryT>(
        "TaggerTrackerClusterPositions",
        {"TaggerTrackerRawHits"},
        {"TaggerTrackerClusterPositions"},
        {
          .readout = "TaggerTrackerHits"
        },
        app
    ));

    // Reconstrution of tracks on common plane
    app->Add(new JOmniFactoryGeneratorT<FarDetectorLinearTracking_factoryT>("LowQ2Tracks",{"TaggerTrackerClusterPositions"},{"LowQ2Tracks"}, tracking_cfg, app));

    // Reconstrution of tracks on common plane
    app->Add(new JOmniFactoryGeneratorT<FarDetectorLinearProjection_factoryT>("LowQ2Projections",{"LowQ2Tracks"},{"LowQ2Projections"}, projection_cfg, app));

    // Vector reconstruction at origin
    app->Add(new JOmniFactoryGeneratorT<FarDetectorMLReconstruction_factoryT>("LowQ2Trajectories",{"LowQ2Projections"},{"LowQ2Trajectories","LowQ2TrackParameters"}, recon_cfg, app));

  }
}
