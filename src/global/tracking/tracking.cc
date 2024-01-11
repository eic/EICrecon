// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <algorithm>
#include <string>

#include "CKFTrackingConfig.h"
#include "CKFTracking_factory.h"
#include "IterativeVertexFinder_factory.h"
#include "TrackParamTruthInit_factory.h"
#include "TrackProjector_factory.h"
#include "TrackPropagation_factory.h"
#include "TrackSeeding_factory.h"
#include "TrackerMeasurementFromHits_factory.h"
#include "extensions/jana/JChainMultifactoryGeneratorT.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/tracking/TrackerHitCollector_factory.h"

//
extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    app->Add(new JOmniFactoryGeneratorT<TrackParamTruthInit_factory>(
            "InitTrackParams",
            {"MCParticles"},
            {"InitTrackParams"},
            {},
            app
            ));

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

    app->Add(new JOmniFactoryGeneratorT<TrackerMeasurementFromHits_factory>(
            "CentralTrackerMeasurements",
            {"CentralTrackingRecHits"},
            {"CentralTrackerMeasurements"},
            app
            ));

    app->Add(new JOmniFactoryGeneratorT<CKFTracking_factory>(
        "CentralCKFTrajectories",
        {
            "InitTrackParams",
            "CentralTrackerMeasurements"
        },
        {
            "CentralCKFTrajectories",
            "CentralCKFTrackParameters",
            "CentralCKFActsTrajectories",
            "CentralCKFActsTracks",
        },
        app
    ));

    app->Add(new JOmniFactoryGeneratorT<TrackSeeding_factory>(
        "CentralTrackSeedingResults",
        {"CentralTrackingRecHits"},
        {"CentralTrackSeedingResults"},
        {},
        app
        ));

    app->Add(new JOmniFactoryGeneratorT<CKFTracking_factory>(
        "CentralCKFSeededTrajectories",
        {
            "CentralTrackSeedingResults",
            "CentralTrackerMeasurements"
        },
        {
            "CentralCKFSeededTrajectories",
            "CentralCKFSeededTrackParameters",
            "CentralCKFSeededActsTrajectories",
            "CentralCKFSeededActsTracks",
        },
        app
    ));

    app->Add(new JOmniFactoryGeneratorT<TrackProjector_factory>(
            "CentralTrackSegments",
            {"CentralCKFActsTrajectories"},
            {"CentralTrackSegments"},
            app
            ));

    app->Add(new JOmniFactoryGeneratorT<IterativeVertexFinder_factory>(
            "CentralTrackVertices",
            {"CentralCKFActsTrajectories"},
            {"CentralTrackVertices"},
            {},
            app
            ));

    app->Add(new JChainMultifactoryGeneratorT<TrackPropagation_factory>(
            "CalorimeterTrackPropagator",
            {"CentralCKFActsTrajectories", "CentralCKFActsTracks"},
            {"CalorimeterTrackProjections"},
            app
            ));

}
} // extern "C"
