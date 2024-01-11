// Copyright 2023, Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <string>

#include "factories/digi/SiliconTrackerDigi_factory.h"
#include "factories/fardetectors/FarDetectorTrackerCluster_factory.h"
#include "factories/fardetectors/FarDetectorLinearTracking_factory.h"
#include "factories/fardetectors/FarDetectorLinearProjection_factory.h"
#include "factories/fardetectors/FarDetectorMLReconstruction_factory.h"

#include "extensions/jana/JOmniFactoryGeneratorT.h"

extern "C" {
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    std::string tracker_readout = "TaggerTrackerHits";


//     FarDetectorLinearProjectionConfig projection_cfg;

//     FarDetectorMLReconstructionConfig recon_cfg;

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
    app->Add(new JOmniFactoryGeneratorT<FarDetectorTrackerCluster_factory>(
        "TaggerTrackerClusterPositions",
        {"TaggerTrackerRawHits"},
        {"TaggerTrackerClusterPositions"},
        {
          .readout = tracker_readout,
        },
        app
    ));

    // Reconstrution of tracks on common plane
    app->Add(new JOmniFactoryGeneratorT<FarDetectorLinearTracking_factory>(
        "LowQ2Tracks",
        {"TaggerTrackerClusterPositions"},
        {"LowQ2Tracks"},
        {
          .readout = tracker_readout,
        },
        app
    ));

    // Reconstrution of tracks on common plane
    app->Add(new JOmniFactoryGeneratorT<FarDetectorLinearProjection_factory>(
        "LowQ2Projections",
        {"LowQ2Tracks"},
        {"LowQ2Projections"},
        {},
        app
    ));

    // Reconstrution of tracks on common plane
    app->Add(new JOmniFactoryGeneratorT<FarDetectorLinearProjection_factory>(
        "LowQ2Projections2",
        {"LowQ2Tracks"},
        {"LowQ2Projections2"},
        {
	  .plane_position = {0.0,0.0,-14.865},
	  .plane_a        = {1.0,0.0,0.0    },
	  .plane_b        = {0.0,1.0,0.0    },
	},
        app
    ));

    // Vector reconstruction at origin
    app->Add(new JOmniFactoryGeneratorT<FarDetectorMLReconstruction_factory>(
        "LowQ2Trajectories",
        {"LowQ2Projections"},
        {"LowQ2Trajectories","LowQ2TrackParameters"},
        {},
        app
    ));

  }
}
