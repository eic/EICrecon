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
#include "factories/meta/CollectionCollector_factory.h"

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

    // Possible collections from arches, brycecanyon and craterlake configurations
    std::vector<std::pair<std::string, std::string>> possible_collections = {
        {"SiBarrelHits", "SiBarrelTrackerRecHits"},
        {"VertexBarrelHits", "SiBarrelVertexRecHits"},
        {"TrackerEndcapHits", "SiEndcapTrackerRecHits"},
        {"TOFBarrelHits", "TOFBarrelRecHit"},
        {"TOFEndcapHits", "TOFEndcapRecHits"},
        {"MPGDBarrelHits", "MPGDBarrelRecHits"},
        {"MPDGDIRCHits", "MPDGDIRCRecHits"},
        {"OuterMPGDBarrelHits", "OuterMPGDBarrelRecHits"},
        {"BackwardMPGDEndcapHits", "BackwardMPGDEndcapRecHits"},
        {"ForwardMPGDEndcapHits", "ForwardMPGDEndcapRecHits"},
        {"B0TrackerHits", "B0TrackerRecHits"}
    };

    // Filter out collections that are not present in the current configuration
    std::vector<std::string> input_collections;
    auto readouts = app->GetService<DD4hep_service>()->detector()->readouts();
    for (const auto& [hit_collection, rec_collection] : possible_collections) {
        if (readouts.find(hit_collection) != readouts.end()) {
            // Add the collection to the list of input collections
            input_collections.push_back(rec_collection);
        }
    }

    // Tracker hits collector
    app->Add(new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::TrackerHit>>(
        "CentralTrackingRecHits",
        input_collections,
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
            "CentralCKFTracks",
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
            "CentralCKFSeededTracks",
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

    // Tracker trajectory collector from ACTS and other factories
    app->Add(new JChainMultifactoryGeneratorT<TrackerTrajectoryCollector_factory>(
         "CombinedTrajectories",
         {
            "CentralCKFTrajectories",  // ACTS output
            "LowQ2Trajectories"        // Low Q2 output
         },
         {"CombinedTrajectories"},
         app
    ));    // Output collection name

    // Tracker trajectory collector from ACTS and other factories
    app->Add(new JChainMultifactoryGeneratorT<TrackerTrajectoryCollector_factory>(
         "CombinedSeededTrajectories",
         {
            "CentralCKFSeededTrajectories",  // ACTS output
            "LowQ2Trajectories"              // Low Q2 output
         },
         {"CombinedSeededTrajectories"},
         app
    ));  // Output collection name

    app->Add(new JChainMultifactoryGeneratorT<TrackPropagation_factory>(
            "CalorimeterTrackPropagator",
            {"CentralCKFActsTrajectories", "CentralCKFActsTracks"},
            {"CalorimeterTrackProjections"},
            app
            ));

}
} // extern "C"
