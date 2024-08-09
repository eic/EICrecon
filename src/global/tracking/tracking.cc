// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2024, Dmitry Romanov, Tyler Kutz, Wouter Deconinck

#include <DD4hep/Detector.h>
#include <JANA/JApplication.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <algorithm>
#include <gsl/pointers>
#include <map>
#include <memory>
#include <string>
#include <utility>
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
#include "TracksToParticlesConfig.h"
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
    std::vector<std::pair<std::string, std::string>> possible_collections = {
        {"SiBarrelHits", "SiBarrelTrackerRecHits"},
        {"VertexBarrelHits", "SiBarrelVertexRecHits"},
        {"TrackerEndcapHits", "SiEndcapTrackerRecHits"},
        {"TOFBarrelHits", "TOFBarrelRecHit"},
        {"TOFEndcapHits", "TOFEndcapRecHits"},
        {"MPGDBarrelHits", "MPGDBarrelRecHits"},
        {"MPGDDIRCHits", "MPGDDIRCRecHits"},
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
        },
        {
            "CentralCKFTrajectoriesUnfiltered",
            "CentralCKFTrackParametersUnfiltered",
            "CentralCKFTracksUnfiltered",
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
        },
        {
            "CentralCKFTrajectories",
            "CentralCKFTrackParameters",
            "CentralCKFTracks",
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
            "CentralCKFSeededActsTrajectoriesUnfiltered",
            "CentralCKFSeededActsTracksUnfiltered",
        },
        app
    ));

    app->Add(new JOmniFactoryGeneratorT<ActsToTracks_factory>(
        "CentralCKFSeededTracksUnfiltered",
        {
            "CentralTrackerMeasurements",
            "CentralCKFSeededActsTrajectoriesUnfiltered",
        },
        {
            "CentralCKFSeededTrajectoriesUnfiltered",
            "CentralCKFSeededTrackParametersUnfiltered",
            "CentralCKFSeededTracksUnfiltered",
        },
        app
    ));

    app->Add(new JOmniFactoryGeneratorT<AmbiguitySolver_factory>(
        "SeededAmbiguityResolutionSolver",
        {
             "CentralCKFSeededActsTracksUnfiltered",
             "CentralTrackerMeasurements"
        },
        {
             "CentralCKFSeededActsTracks",
             "CentralCKFSeededActsTrajectories",
        },
        app
    ));

    app->Add(new JOmniFactoryGeneratorT<ActsToTracks_factory>(
        "CentralCKFSeededTracks",
        {
            "CentralTrackerMeasurements",
            "CentralCKFSeededActsTrajectories",
        },
        {
            "CentralCKFSeededTrajectories",
            "CentralCKFSeededTrackParameters",
            "CentralCKFSeededTracks",
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
    //Check size of input_collections to determine if CentralCKFTracks should be added to the input_track_collections
    if (input_collections.size() > 0) {
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

     // linking of reconstructed particles to PID objects
     TracksToParticlesConfig link_cfg {
       .momentumRelativeTolerance = 100.0, /// Matching momentum effectively disabled
       .phiTolerance              = 0.1, /// Matching phi tolerance [rad]
       .etaTolerance              = 0.2, /// Matching eta tolerance
     };

     app->Add(new JOmniFactoryGeneratorT<TracksToParticles_factory>(
             "ChargedParticlesWithAssociations",
             {"MCParticles",                                    // edm4hep::MCParticle
             "CombinedTracks",                                // edm4eic::Track
             },
             {"ReconstructedChargedWithoutPIDParticles",                  //
              "ReconstructedChargedWithoutPIDParticleAssociations"        // edm4eic::MCRecoParticleAssociation
             },
             link_cfg,
             app
             ));

     app->Add(new JOmniFactoryGeneratorT<TracksToParticles_factory>(
             "ChargedSeededParticlesWithAssociations",
             {"MCParticles",                                    // edm4hep::MCParticle
             "CentralCKFSeededTracks",                          // edm4eic::Track
             },
             {"ReconstructedSeededChargedWithoutPIDParticles",            //
              "ReconstructedSeededChargedWithoutPIDParticleAssociations"  // edm4eic::MCRecoParticleAssociation
             },
             link_cfg,
             app
             ));
}
} // extern "C"
