// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2024, Dmitry Romanov, Tyler Kutz, Wouter Deconinck, Dmitry Kalinkin

#include <DD4hep/Detector.h>
#include <JANA/JApplication.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <algorithm>
#include <gsl/pointers>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "ActsToTracks.h"
#include "ActsToTracks_factory.h"
#include "AmbiguitySolver_factory.h"
#include "CKFTracking_factory.h"
#include "IterativeVertexFinder_factory.h"
#include "TrackParamTruthInit_factory.h"
#include "TrackProjector_factory.h"
#include "TrackPropagationConfig.h"
#include "TrackPropagation_factory.h"
#include "TrackSeeding_factory.h"
#include "TrackerMeasurementFromHits_factory.h"
#include "TracksToParticles_factory.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/meta/CollectionCollector_factory.h"
#include "services/geometry/dd4hep/DD4hep_service.h"

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
    std::vector<std::tuple<std::string, std::string, std::string, std::string>> possible_collections = {
        {"SiBarrelHits", "SiBarrelRawHits", "SiBarrelRawHitAssociations", "SiBarrelTrackerRecHits"},
        {"VertexBarrelHits", "SiBarrelVertexRawHits", "SiBarrelVertexRawHitAssociations", "SiBarrelVertexRecHits"},
        {"TrackerEndcapHits", "SiEndcapTrackerRawHits", "SiEndcapTrackerRawHitAssociations", "SiEndcapTrackerRecHits"},
        {"TOFBarrelHits", "TOFBarrelRawHits", "TOFBarrelRawHitAssociations", "TOFBarrelRecHit"},
        {"TOFEndcapHits", "TOFEndcapRawHits", "TOFEndcapRawHitAssociations", "TOFEndcapRecHits"},
        {"MPGDBarrelHits", "MPGDBarrelRawHits", "MPGDBarrelRawHitAssociations", "MPGDBarrelRecHits"},
        {"OuterMPGDBarrelHits", "OuterMPGDBarrelRawHits", "OuterMPGDBarrelRawHitAssociations", "OuterMPGDBarrelRecHits"},
        {"BackwardMPGDEndcapHits", "BackwardMPGDEndcapRawHits", "BackwardMPGDEndcapRawHitAssociations", "BackwardMPGDEndcapRecHits"},
        {"ForwardMPGDEndcapHits", "ForwardMPGDEndcapRawHits", "ForwardMPGDEndcapRawHitAssociations", "ForwardMPGDEndcapRecHits"},
        {"B0TrackerHits", "B0TrackerRawHits", "B0TrackerRawHitAssociations", "B0TrackerRecHits"}
    };

    // Filter out collections that are not present in the current configuration
    std::vector<std::string> input_rec_collections;
    std::vector<std::string> input_raw_assoc_collections;
    auto readouts = app->GetService<DD4hep_service>()->detector()->readouts();
    for (const auto& [hit_collection, raw_collection, raw_assoc_collection, rec_collection] : possible_collections) {
      if (readouts.find(hit_collection) != readouts.end()) {
        // Add the collection to the list of input collections
        input_rec_collections.push_back(rec_collection);
        input_raw_assoc_collections.push_back(raw_assoc_collection);
      }
    }

    // Tracker hits collector
    app->Add(new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::TrackerHit>>(
        "CentralTrackingRecHits",
        input_rec_collections,
        {"CentralTrackingRecHits"}, // Output collection name
        app));

    // Tracker hit associations collector
    app->Add(new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::MCRecoTrackerHitAssociation>>(
        "CentralTrackingRawHitAssociations",
        input_raw_assoc_collections,
        {"CentralTrackingRawHitAssociations"}, // Output collection name
        app));

    app->Add(new JOmniFactoryGeneratorT<TrackerMeasurementFromHits_factory>(
            "CentralTrackerMeasurements",
            {"CentralTrackingRecHits"},
            {"CentralTrackerMeasurements"},
            app
            ));

    app->Add(new JOmniFactoryGeneratorT<CKFTracking_factory>(
        "CentralCKFTruthSeededTrajectories",
        {
            "InitTrackParams",
            "CentralTrackerMeasurements"
        },
        {
            "CentralCKFTruthSeededActsTrajectoriesUnfiltered",
            "CentralCKFTruthSeededActsTracksUnfiltered",
        },
        app
    ));

    app->Add(new JOmniFactoryGeneratorT<ActsToTracks_factory>(
        "CentralCKFTruthSeededTracksUnfiltered",
        {
            "CentralTrackerMeasurements",
            "CentralCKFTruthSeededActsTrajectoriesUnfiltered",
            "CentralTrackingRawHitAssociations",
        },
        {
            "CentralCKFTruthSeededTrajectoriesUnfiltered",
            "CentralCKFTruthSeededTrackParametersUnfiltered",
            "CentralCKFTruthSeededTracksUnfiltered",
            "CentralCKFTruthSeededTrackUnfilteredAssociations",
        },
        app
    ));

    app->Add(new JOmniFactoryGeneratorT<AmbiguitySolver_factory>(
        "TruthSeededAmbiguityResolutionSolver",
        {
             "CentralCKFTruthSeededActsTracksUnfiltered",
             "CentralTrackerMeasurements"
        },
        {
             "CentralCKFTruthSeededActsTracks",
             "CentralCKFTruthSeededActsTrajectories",
        },
        app
    ));

    app->Add(new JOmniFactoryGeneratorT<ActsToTracks_factory>(
        "CentralCKFTruthSeededTracks",
        {
            "CentralTrackerMeasurements",
            "CentralCKFTruthSeededActsTrajectories",
            "CentralTrackingRawHitAssociations",
        },
        {
            "CentralCKFTruthSeededTrajectories",
            "CentralCKFTruthSeededTrackParameters",
            "CentralCKFTruthSeededTracks",
            "CentralCKFTruthSeededTrackAssociations",
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
        "CentralCKFTrajectories",
        {
            "CentralTrackSeedingResults",
            "CentralTrackerMeasurements"
        },
        {
            "CentralCKFActsTrajectoriesUnfiltered",
            "CentralCKFActsTracksUnfiltered",
        },
        app
    ));

    app->Add(new JOmniFactoryGeneratorT<ActsToTracks_factory>(
        "CentralCKFTracksUnfiltered",
        {
            "CentralTrackerMeasurements",
            "CentralCKFActsTrajectoriesUnfiltered",
            "CentralTrackingRawHitAssociations",
        },
        {
            "CentralCKFTrajectoriesUnfiltered",
            "CentralCKFTrackParametersUnfiltered",
            "CentralCKFTracksUnfiltered",
            "CentralCKFTrackUnfilteredAssociations",
        },
        app
    ));

    app->Add(new JOmniFactoryGeneratorT<AmbiguitySolver_factory>(
        "AmbiguityResolutionSolver",
        {
             "CentralCKFActsTracksUnfiltered",
             "CentralTrackerMeasurements"
        },
        {
             "CentralCKFActsTracks",
             "CentralCKFActsTrajectories",
        },
        app
    ));

    app->Add(new JOmniFactoryGeneratorT<ActsToTracks_factory>(
        "CentralCKFTracks",
        {
            "CentralTrackerMeasurements",
            "CentralCKFActsTrajectories",
            "CentralTrackingRawHitAssociations",
        },
        {
            "CentralCKFTrajectories",
            "CentralCKFTrackParameters",
            "CentralCKFTracks",
            "CentralCKFTrackAssociations",
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
            {"CentralCKFActsTrajectories","ReconstructedChargedParticles"},
            {"CentralTrackVertices"},
            {},
            app
            ));

    app->Add(new JOmniFactoryGeneratorT<TrackPropagation_factory>(
            "CalorimeterTrackPropagator",
            {"CentralCKFTracks", "CentralCKFActsTrajectories", "CentralCKFActsTracks"},
            {"CalorimeterTrackProjections"},
            {
                .target_surfaces{
                    // Ecal
                    eicrecon::DiscSurfaceConfig{"EcalEndcapN_ID", "- EcalEndcapN_zmin", 0., "1.1*EcalEndcapN_rmax"},
                    eicrecon::DiscSurfaceConfig{"EcalEndcapN_ID", "- EcalEndcapN_zmin - 50*mm", 0., "1.1*EcalEndcapN_rmax"},
                    eicrecon::CylinderSurfaceConfig{"EcalBarrel_ID", "EcalBarrel_rmin",
                        "- 1.1*max(EcalBarrelBackward_zmax,EcalBarrelForward_zmax)",
                        "1.1*max(EcalBarrelBackward_zmax,EcalBarrelForward_zmax)"
                    },
                    eicrecon::CylinderSurfaceConfig{"EcalBarrel_ID", "EcalBarrel_rmin + 50*mm",
                        "- 1.1*max(EcalBarrelBackward_zmax,EcalBarrelForward_zmax)",
                        "1.1*max(EcalBarrelBackward_zmax,EcalBarrelForward_zmax)"
                    },
                    eicrecon::DiscSurfaceConfig{"EcalEndcapP_ID", "EcalEndcapP_zmin", 0., "1.1*EcalEndcapP_rmax"},
                    eicrecon::DiscSurfaceConfig{"EcalEndcapP_ID", "EcalEndcapP_zmin + 50*mm", 0., "1.1*EcalEndcapP_rmax"},
                    // Hcal
                    eicrecon::DiscSurfaceConfig{"HcalEndcapN_ID", "- HcalEndcapN_zmin", 0., "1.1*HcalEndcapN_rmax"},
                    eicrecon::DiscSurfaceConfig{"HcalEndcapN_ID", "- HcalEndcapN_zmin - 150*mm", 0., "1.1*HcalEndcapN_rmax"},
                    eicrecon::CylinderSurfaceConfig{"HcalBarrel_ID", "HcalBarrel_rmin",
                        "- 1.1*max(HcalBarrelBackward_zmax,HcalBarrelForward_zmax)",
                        "1.1*max(HcalBarrelBackward_zmax,HcalBarrelForward_zmax)"
                    },
                    eicrecon::CylinderSurfaceConfig{"HcalBarrel_ID", "HcalBarrel_rmin + 150*mm",
                        "- 1.1*max(HcalBarrelBackward_zmax,HcalBarrelForward_zmax)",
                        "1.1*max(HcalBarrelBackward_zmax,HcalBarrelForward_zmax)"
                    },
                    eicrecon::DiscSurfaceConfig{"LFHCAL_ID", "LFHCAL_zmin", 0., "1.1*LFHCAL_rmax"},
                    eicrecon::DiscSurfaceConfig{"LFHCAL_ID", "LFHCAL_zmin + 150*mm", 0., "1.1*LFHCAL_rmax"},
                }
            },
            app
            ));



    std::vector<std::string> input_track_collections;
    //Check size of input_rec_collections to determine if CentralCKFTracks should be added to the input_track_collections
    if (input_rec_collections.size() > 0) {
        input_track_collections.push_back("CentralCKFTracks");
    }
    //Check if the TaggerTracker readout is present in the current configuration
    if (readouts.find("TaggerTrackerHits") != readouts.end()) {
        input_track_collections.push_back("TaggerTrackerTracks");
    }

    // Add central and other tracks
    app->Add(new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::Track>>(
            "CombinedTracks",
            input_track_collections,
            {"CombinedTracks"},
            app
            ));

    app->Add(new JOmniFactoryGeneratorT<TracksToParticles_factory>(
            "ChargedTruthSeededParticlesWithAssociations",
            {
              "CentralCKFTruthSeededTracks",
              "CentralCKFTruthSeededTrackAssociations",
            },
            {"ReconstructedTruthSeededChargedWithoutPIDParticles",
             "ReconstructedTruthSeededChargedWithoutPIDParticleAssociations"
            },
            {},
            app
            ));

    app->Add(new JOmniFactoryGeneratorT<TracksToParticles_factory>(
            "ChargedParticlesWithAssociations",
            {
              "CombinedTracks",
              "CentralCKFTrackAssociations",
            },
            {
              "ReconstructedChargedWithoutPIDParticles",
              "ReconstructedChargedWithoutPIDParticleAssociations"
            },
            {},
            app
            ));
}
} // extern "C"
