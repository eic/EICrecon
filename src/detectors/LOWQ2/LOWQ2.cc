// Copyright 2023, Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Acts/Propagator/Navigator.hpp>

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>
#include <JANA/JEvent.h>

#include <extensions/jana/JChainFactoryGeneratorT.h>

#include "extensions/jana/JChainMultifactoryGeneratorT.h"

//#include "LowQ2ProtoCluster_factory.h"
//#include "LowQ2Cluster_factory.h"
// #include "LowQ2Tracking_factory.h"
// #include "LowQ2Reconstruction_factory.h"

#include <factories/digi/SiliconTrackerDigi_factoryT.h>
#include <global/tracking/TrackerHitReconstruction_factory.h>
//#include <global/fardetectors/FarDetectorProtoCluster_factory.h>
#include <global/fardetectors/FarDetectorCluster_factory.h>
#include <global/fardetectors/FarDetectorSimpleTracking_factory.h>
#include <global/fardetectors/FarDetectorMLReconstruction_factory.h>


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
    FarTrackerClusterConfig cluster_cfg;
    //Ensure same detector is passed to digi and clustering
    cluster_cfg.readout = "TaggerTrackerHits";

    FarTrackerTrackingConfig tracking_cfg;
    tracking_cfg.detconf = cluster_cfg;

    //Why isn't there the same for energy digitization, just std::llround(sim_hit->getEDep() * 1e6)? Whole Digi process isn't quite consistent.
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
    //    app->Add(new JChainFactoryGeneratorT<LowQ2ProtoCluster_factory>({"TaggerTrackerRawHit"},    "TaggerTrackerProtoClusters" ));
    app->Add(new JChainFactoryGeneratorT<FarDetectorCluster_factory>({"TaggerTrackerRawHit"},    "TaggerTrackerClusterPositions", cluster_cfg));

    // Cluster position generation
    //    app->Add(new JChainFactoryGeneratorT<FarDetectorCluster_factory>({"TaggerTrackerProtoClusters"}, "TaggerTrackerClusterPositions"));
    //    app->Add(new JChainFactoryGeneratorT<LowQ2Cluster_factory>({"TaggerTrackerProtoClusters"}, "TaggerTrackerClusterPositions"));

    // Very basic reconstrution of a single track
    app->Add(new JChainFactoryGeneratorT<FarDetectorSimpleTracking_factory>({"TaggerTrackerClusterPositions"},"LowQ2Tracks", tracking_cfg));

    // Initial particle reconstruction
    app->Add(new JChainFactoryGeneratorT<FarDetectorMLReconstruction_factory>({"LowQ2Tracks"},"LowQ2Particles"));

  }
}
