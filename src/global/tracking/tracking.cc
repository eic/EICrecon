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

    app->Add(new JOmniFactoryGeneratorT<TrackPropagation_factory>(
            "CalorimeterTrackPropagator",
            {"CentralCKFActsTrajectories", "CentralCKFActsTracks"},
            {"CalorimeterTrackProjections"},
            {
                {
                    // Ecal
                    eicrecon::DiscSurfaceConfig{"EcalEndcapN_ID", "- EcalEndcapN_zmin", "EcalEndcapN_rmin", "1.1*EcalEndcapN_rmax"},
                    eicrecon::DiscSurfaceConfig{"EcalEndcapN_ID", "- EcalEndcapN_zmin - 50*cm", "EcalEndcapN_rmin", "1.1*EcalEndcapN_rmax"},
                    eicrecon::CylinderSurfaceConfig{"EcalBarrel_ID", "EcalBarrel_rmin", "- 1.1*EcalBarrelBackward_zmin", "1.1*EcalBarrelForward_zmax"},
                    eicrecon::CylinderSurfaceConfig{"EcalBarrel_ID", "EcalBarrel_rmin + 50*cm", "- 1.1*EcalBarrelBackward_zmin", "1.1*EcalBarrelForward_zmax"},
                    eicrecon::DiscSurfaceConfig{"EcalEndcapP_ID", "EcalEndcapP_zmin", "EcalEndcapP_rmin", "1.1*EcalEndcapP_rmax"},
                    eicrecon::DiscSurfaceConfig{"EcalEndcapP_ID", "EcalEndcapP_zmin + 50*cm", "EcalEndcapP_rmin", "1.1*EcalEndcapP_rmax"},
                    // Hcal
                    eicrecon::DiscSurfaceConfig{"HcalEndcapN_ID", "- HcalEndcapN_zmin", "HcalEndcapN_rmin", "1.1*HcalEndcapN_rmax"},
                    eicrecon::DiscSurfaceConfig{"HcalEndcapN_ID", "- HcalEndcapN_zmin - 150*cm", "HcalEndcapN_rmin", "1.1*HcalEndcapN_rmax"},
                    eicrecon::CylinderSurfaceConfig{"HcalBarrel_ID", "HcalBarrel_rmin", "- 1.1*HcalBarrelBackward_zmin", "1.1*HcalBarrelForward_zmax"},
                    eicrecon::CylinderSurfaceConfig{"HcalBarrel_ID", "HcalBarrel_rmin + 150*cm", "- 1.1*HcalBarrelBackward_zmin", "1.1*HcalBarrelForward_zmax"},
                    eicrecon::DiscSurfaceConfig{"LFHCAL_ID", "LFHCAL_zmin", "LFHCAL_rmin", "1.1*LFHCAL_rmax"},
                    eicrecon::DiscSurfaceConfig{"LFHCAL_ID", "LFHCAL_zmin + 150*cm", "LFHCAL_rmin", "1.1*LFHCAL_rmax"},
                }
            },
            app
            ));

}
} // extern "C"
