// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Acts/Propagator/Navigator.hpp>

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include "extensions/jana/JChainFactoryGeneratorT.h"
#include "extensions/jana/JChainMultifactoryGeneratorT.h"

#include "factories/tracking/TrackerHitCollector_factory.h"

#include "TrackerSourceLinker_factory.h"
#include "TrackParamTruthInit_factory.h"
#include "CKFTracking_factory.h"
#include "TrackSeeding_factory.h"
#include "TrackProjector_factory.h"
#include "TrackSeeding_factory.h"
#include "ParticlesWithTruthPID_factory.h"
#include "IterativeVertexFinder_factory.h"

//
extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    app->Add(new JChainFactoryGeneratorT<TrackParamTruthInit_factory>(
            {"MCParticles"}, "InitTrackParams"));

    // Tracker hits collector
    app->Add(new JChainMultifactoryGeneratorT<TrackerHitCollector_factory>(
        "CentralTrackingRecHits",
        {
            "SiBarrelTrackerRecHits",          // Si tracker hits
            "SiBarrelVertexRecHits",
            "SiEndcapTrackerRecHits",
            "TOFBarrelRecHit",             // TOF hits
            "TOFEndcapRecHits",
            "MPGDBarrelRecHits",           // MPGD
            "MPGDDIRCRecHits",
            "OuterMPGDBarrelRecHits",
            "BackwardMPGDEndcapRecHits",
            "ForwardMPGDEndcapRecHits",
            "B0TrackerRecHits"          // B0TRK
        },
        {"CentralTrackingRecHits"}, // Output collection name
        app));

    // Source linker
    app->Add(new JChainFactoryGeneratorT<TrackerSourceLinker_factory>(
            {"CentralTrackingRecHits"}, "CentralTrackerSourceLinker"));

    app->Add(new JChainMultifactoryGeneratorT<CKFTracking_factory>(
        "CentralCKFTrajectories",
        {
            "InitTrackParams",
            "CentralTrackerSourceLinker"
        },
        {
            "CentralCKFTrajectories",
            "CentralCKFActsTrajectories",
        },
        app
    ));

    app->Add(new JChainFactoryGeneratorT<TrackSeeding_factory>(
            {"CentralTrackingRecHits"}, "CentralTrackSeedingResults"));

    app->Add(new JChainMultifactoryGeneratorT<CKFTracking_factory>(
        "CentralCKFSeededTrajectories",
        {
            "CentralTrackSeedingResults",
            "CentralTrackerSourceLinker"
        },
        {
            "CentralCKFSeededTrajectories",
            "CentralCKFSeededActsTrajectories",
        },
        app
    ));

    app->Add(new JChainFactoryGeneratorT<TrackProjector_factory>(
            {"CentralCKFActsTrajectories"}, "CentralTrackSegments"));

    app->Add(new JChainFactoryGeneratorT<IterativeVertexFinder_factory>(
            {"CentralCKFActsTrajectories"}, "CentralTrackVertices"));

    app->Add(new JChainMultifactoryGeneratorT<ParticlesWithTruthPID_factory>(
            "ChargedParticlesWithAssociations",                // Tag name for multifactory
            {"MCParticles",                                    // edm4hep::MCParticle
            "CentralCKFTrajectories"},                         // edm4eic::Trajectory
            {"ReconstructedChargedParticles",                  //
             "ReconstructedChargedParticleAssociations"        // edm4eic::MCRecoParticleAssociation
            },
            app  // TODO: Remove me once fixed
            ));

    app->Add(new JChainMultifactoryGeneratorT<ParticlesWithTruthPID_factory>(
            "ChargedParticlesWithAssociations",                // Tag name for multifactory
            {"MCParticles",                                    // edm4hep::MCParticle
            "CentralCKFSeededTrajectories"},                   // edm4eic::Trajectory
            {"ReconstructedSeededChargedParticles",            //
             "ReconstructedSeededChargedParticleAssociations"  // edm4eic::MCRecoParticleAssociation
            },
            app  // TODO: Remove me once fixed
            ));

}
} // extern "C"
