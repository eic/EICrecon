// Copyright 2023, Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Acts/Propagator/Navigator.hpp>

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>
#include <JANA/JEvent.h>

#include <extensions/jana/JChainFactoryGeneratorT.h>

//#include "LowQ2ProtoCluster_factory.h"
#include "LowQ2Cluster_factory.h"
#include "LowQ2Tracking_factory.h"
#include "LowQ2Reconstruction_factory.h"

#include <global/digi/SiliconTrackerDigi_factory.h>
#include <global/tracking/TrackerHitReconstruction_factory.h>
#include "global/fardetectors/FarDetectorProtoCluster_factory.h"


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
    TrackerClusterConfig cluster_cfg;
    //Ensure same detector is passed to digi and clustering
    cluster_cfg.readout = "TaggerTrackerHits";

    //Why isn't there the same for energy digitization, just std::llround(sim_hit->getEDep() * 1e6)? Whole Digi process isn't quite consistent.
    app->Add(new JChainFactoryGeneratorT<SiliconTrackerDigi_factory>({cluster_cfg.readout},      "TaggerTrackerRawHit", digi_cfg));

    
    // Clustering of hits
    //    app->Add(new JChainFactoryGeneratorT<LowQ2ProtoCluster_factory>({"TaggerTrackerRawHit"},    "TaggerTrackerProtoClusters" ));
    app->Add(new JChainFactoryGeneratorT<FarDetectorProtoCluster_factory>({"TaggerTrackerRawHit"},    "TaggerTrackerProtoClusters", cluster_cfg));

    // Cluster position generation
    app->Add(new JChainFactoryGeneratorT<LowQ2Cluster_factory>({"TaggerTrackerProtoClusters"}, "TaggerTrackerClusterPositions"));

    // Very basic reconstrution of a single track
    app->Add(new JChainFactoryGeneratorT<LowQ2Tracking_factory>({"TaggerTrackerClusterPositions"},"LowQ2Tracks"));

    // Initial particle reconstruction
    app->Add(new JChainFactoryGeneratorT<LowQ2Reconstruction_factory>({"LowQ2Tracks"},"LowQ2Particles"));

  }
}
