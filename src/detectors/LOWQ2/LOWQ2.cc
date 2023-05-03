// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Acts/Propagator/Navigator.hpp>

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>
#include <JANA/JEvent.h>

#include <extensions/jana/JChainFactoryGeneratorT.h>

#include <global/digi/SiliconTrackerDigi_factory.h>
#include <global/tracking/TrackerHitReconstruction_factory.h>

#include <global/tracking/TrackerSourceLinker_factory.h>
#include <global/tracking/TrackParamTruthInit_factory.h>
#include <global/tracking/TrackingResult_factory.h>
#include <global/tracking/TrackerReconstructedParticle_factory.h>
#include <global/tracking/TrackParameters_factory.h>
#include <global/tracking/CKFTracking_factory.h>
#include <global/tracking/TrackSeeding_factory.h>
#include <global/tracking/TrackerHitCollector_factory.h>
#include <global/tracking/TrackParameters_factory.h>
#include <global/tracking/TrackProjector_factory.h>
#include <global/tracking/ParticlesWithTruthPID_factory.h>
#include <global/reco/ReconstructedParticles_factory.h>
#include <global/reco/ReconstructedParticleAssociations_factory.h>

extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;


    // Digitization
    SiliconTrackerDigiConfig digi_cfg;
    //digi_cfg.timeResolution = 2.5; // Change timing resolution.
    //Why isn't there the same for energy digitization, just std::llround(sim_hit->getEDep() * 1e6)? Whole Digi process isn't quite consistent.
    app->Add(new JChainFactoryGeneratorT<SiliconTrackerDigi_factory>({"TaggerTrackerHits"}, "TaggerTrackerRawHit", digi_cfg));

    TrackerHitReconstructionConfig hit_reco_cfg;
    // change default parameters like hit_reco_cfg.time_resolution = 10;
    app->Add(new JChainFactoryGeneratorT<TrackerHitReconstruction_factory>({"TaggerTrackerRawHit"}, "TaggerTrackerHit", hit_reco_cfg));




    // Source linker
    app->Add(new JChainFactoryGeneratorT<TrackerSourceLinker_factory>(
            {"TaggerTrackingHits"}, "TaggerTrackerSourceLinker"));

    app->Add(new JChainFactoryGeneratorT<CKFTracking_factory>(
            {"TaggerTrackerSourceLinker"}, "TaggerCKFTrajectories"));

    app->Add(new JChainFactoryGeneratorT<TrackSeeding_factory>(
            {"TaggerTrackingRecHits"}, "TaggerTrackSeedingResults"));

    app->Add(new JChainFactoryGeneratorT<TrackProjector_factory>(
            {"TaggerCKFTrajectories"}, "TaggerTrackSegments"));

    app->Add(new JChainFactoryGeneratorT<TrackingResult_factory>(
            {"TaggerCKFTrajectories"}, "TaggerTrackingParticles"));

    app->Add(new JChainFactoryGeneratorT<TrackerReconstructedParticle_factory>(
            {"TaggerTrackingParticles"}, "TaggerParticles"));

    app->Add(new JChainFactoryGeneratorT<TrackParameters_factory>(
            {"TaggerTrackingParticles"}, "TaggerTrackParameters"));

    app->Add(new JChainFactoryGeneratorT<ParticlesWithTruthPID_factory>(
            {"MCParticles",                         // Tag for edm4hep::MCParticle
            "TaggerTrackParameters"},               // Tag for edm4eic::TrackParameters
            "TaggerParticlesWithAssociations"));   // eicrecon::ParticlesWithAssociation

    app->Add(new JChainFactoryGeneratorT<ReconstructedParticles_factory>(
            {"TaggerParticlesWithAssociations"},
            "ReconstructedTaggerParticles"));

    app->Add(new JChainFactoryGeneratorT<ReconstructedParticleAssociations_factory>(
            {"TaggerParticlesWithAssociations"},
            "ReconstructedTaggerParticlesAssociations"));




//     app->Add(new JFactoryGeneratorT<RawTrackerHit_factory_ForwardRomanPotRawHits>());
//     app->Add(new JFactoryGeneratorT<TrackerHit_factory_ForwardRomanPotRecHits>());
//     app->Add(new JFactoryGeneratorT<ReconstructedParticle_factory_ForwardRomanPotParticles>());
}
}

