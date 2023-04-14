// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Acts/Propagator/Navigator.hpp>

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <extensions/jana/JChainFactoryGeneratorT.h>
#include <extensions/jana/JChainMultifactoryGeneratorT.h>

#include "TrackerSourceLinker_factory.h"
#include "TrackParamTruthInit_factory.h"
#include "TrackingResult_factory.h"
#include "CKFTracking_factory.h"
#include "TrackSeeding_factory.h"
#include "TrackerHitCollector_factory.h"
#include "TrackProjector_factory.h"
#include "ParticlesWithTruthPID_factory.h"

//
extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    app->Add(new JChainFactoryGeneratorT<TrackParamTruthInit_factory>(
            {"MCParticles"}, "InitTrackParams"));

    // Tracker hits collector
    app->Add(new JChainFactoryGeneratorT<TrackerHitCollector_factory>(
                      {
                         "SiBarrelTrackerRecHits",          // Si tracker hits
                         "SiBarrelVertexRecHits",
                         "SiEndcapTrackerRecHits",
                         "TOFBarrelRecHit",             // TOF hits
                         "TOFEndcapRecHits",
                         "MPGDBarrelRecHits",        // MPGD
                         "MPGDDIRCRecHits",
                         "B0TrackerRecHits"          // B0TRK
                      },

                      "CentralTrackingRecHits"));    // Output collection name

    // Source linker
    app->Add(new JChainFactoryGeneratorT<TrackerSourceLinker_factory>(
            {"CentralTrackingRecHits"}, "CentralTrackerSourceLinker"));

    app->Add(new JChainFactoryGeneratorT<CKFTracking_factory>(
            {"CentralTrackerSourceLinker"}, "CentralCKFTrajectories"));

    app->Add(new JChainFactoryGeneratorT<TrackSeeding_factory>(
            {"CentralTrackingRecHits"}, "CentralTrackSeedingResults"));

    app->Add(new JChainFactoryGeneratorT<TrackProjector_factory>(
            {"CentralCKFTrajectories"}, "CentralTrackSegments"));

    app->Add(new JChainMultifactoryGeneratorT<TrackingResult_factory>(
            "CentralTrackingParticles",                       // Tag name for multifactory
            {"CentralCKFTrajectories"},                       // eicrecon::TrackingResultTrajectory
            {"outputParticles",                               // edm4eic::ReconstructedParticle
             "outputTrackParameters"},                        // edm4eic::TrackParameters
            app));

    app->Add(new JChainMultifactoryGeneratorT<ParticlesWithTruthPID_factory>(
            "ChargedParticlesWithAssociations",                // Tag name for multifactory
            {"MCParticles",                                    // edm4hep::MCParticle
            "outputTrackParameters"},                          // edm4eic::TrackParameters
            {"ReconstructedChargedParticles",                  //
             "ReconstructedChargedParticlesAssociations"       // edm4eic::MCRecoParticleAssociation
            },
            app  // TODO: Remove me once fixed
            ));

}
} // extern "C"
