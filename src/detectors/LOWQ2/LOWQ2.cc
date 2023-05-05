// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Acts/Propagator/Navigator.hpp>

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>
#include <JANA/JEvent.h>

#include <extensions/jana/JChainFactoryGeneratorT.h>

#include "LowQ2ProtoCluster_factory.h"
#include "LowQ2Cluster_factory.h"
#include "LowQ2Tracking_factory.h"
#include "LowQ2Reconstruction_factory.h"

#include <global/digi/SiliconTrackerDigi_factory.h>
#include <global/tracking/TrackerHitReconstruction_factory.h>

// #include <global/tracking/TrackerSourceLinker_factory.h>
// #include <global/tracking/TrackParamTruthInit_factory.h>
// #include <global/tracking/TrackingResult_factory.h>
// #include <global/tracking/TrackerReconstructedParticle_factory.h>
// #include <global/tracking/TrackParameters_factory.h>
// #include <global/tracking/CKFTracking_factory.h>
// #include <global/tracking/TrackSeeding_factory.h>
// #include <global/tracking/TrackerHitCollector_factory.h>
// #include <global/tracking/TrackParameters_factory.h>
// #include <global/tracking/TrackProjector_factory.h>
// #include <global/tracking/ParticlesWithTruthPID_factory.h>
// #include <global/reco/ReconstructedParticles_factory.h>
// #include <global/reco/ReconstructedParticleAssociations_factory.h>

extern "C" {
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    
    using namespace eicrecon;
    
    // -------------------------------
    // To do - Parameterization of charge/signal sharing in detector.
    // -------------------------------

    // Digitization
    SiliconTrackerDigiConfig digi_cfg;
    //digi_cfg.timeResolution = 2.5; // Change timing resolution.
    //Why isn't there the same for energy digitization, just std::llround(sim_hit->getEDep() * 1e6)? Whole Digi process isn't quite consistent.
    app->Add(new JChainFactoryGeneratorT<SiliconTrackerDigi_factory>({"TaggerTrackerHits"}, "TaggerTrackerRawHit", digi_cfg));
    
    // -------------------------------
    // To do - Clustering of hits.
    // -------------------------------
    app->Add(new JChainFactoryGeneratorT<LowQ2ProtoCluster_factory>({"TaggerTrackerProtoClusters"},    "TaggerTrackerRawHit"       ));

    app->Add(new JChainFactoryGeneratorT<LowQ2Cluster_factory>({"TaggerTrackerClusterPositions"}, "TaggerTrackerProtoClusters"));
    
    // Convert raw digitized hits into hits with geometry info (ready for tracking)
    TrackerHitReconstructionConfig hit_reco_cfg;
    // change default parameters like hit_reco_cfg.time_resolution = 10;
    app->Add(new JChainFactoryGeneratorT<TrackerHitReconstruction_factory>({"TaggerTrackerRawHit"}, "TaggerTrackerHit", hit_reco_cfg));

    // Very basic reconstrution of a single track
    app->Add(new JFactoryGeneratorT<LowQ2Tracking_factory>());
    //    app->Add(new JFactoryGeneratorT<LowQ2Tracking_factory>("TaggerTrackerHit"),"LowQ2Tracks");

//     // Initial electron reconstruction
//     app->Add(new JFactoryGeneratorT<LowQ2Reconstruction_factory>("LowQ2Tracks"),"LowQ2RecParticles");

  }
}

