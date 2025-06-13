// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025, Dmitry Romanov, Tyler Kutz, Wouter Deconinck, Dmitry Kalinkin

#include <JANA/JApplicationFwd.h>
#include <edm4eic/MCRecoTrackParticleAssociation.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <fmt/core.h>
#include <map>
#include <memory>

#include "algorithms/tracking/ActsToTracks.h"
#include "algorithms/tracking/TrackPropagationConfig.h"
#include "factories/tracking/ActsToTracks_factory.h"
#include "factories/tracking/AmbiguitySolver_factory.h"
#include "factories/tracking/CKFTracking_factory.h"
#include "factories/tracking/IterativeVertexFinder_factory.h"
#include "factories/tracking/SecondaryVertexFinder_factory.h"
#include "factories/tracking/TrackParamTruthInit_factory.h"
#include "factories/tracking/TrackProjector_factory.h"
#include "factories/tracking/TrackPropagation_factory.h"
#include "factories/tracking/TrackSeeding_factory.h"
#include "factories/tracking/TrackerMeasurementFromHits_factory.h"
#include "factories/tracking/TracksToParticles_factory.h"
#include "factories/meta/CollectionCollector_factory.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"

//
extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);

  using namespace eicrecon;

  app->Add(new JOmniFactoryGeneratorT<TrackParamTruthInit_factory>(
      "CentralTrackTruthSeeds", {"MCParticles"}, {"CentralTrackTruthSeeds"}, {}, app));

  // Tracker hits collector
  app->Add(new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::TrackerHit, true>>(
      "CentralTrackingRecHits",
      {"SiBarrelTrackerRecHits", "SiBarrelVertexRecHits", "SiEndcapTrackerRecHits",
       "TOFBarrelRecHits", "TOFEndcapRecHits", "MPGDBarrelRecHits", "OuterMPGDBarrelRecHits",
       "BackwardMPGDEndcapRecHits", "ForwardMPGDEndcapRecHits", "B0TrackerRecHits"},
      {"CentralTrackingRecHits"}, // Output collection name
      app));

  // Tracker hit associations collector
  app->Add(new JOmniFactoryGeneratorT<
           CollectionCollector_factory<edm4eic::MCRecoTrackerHitAssociation, true>>(
      "CentralTrackingRawHitAssociations",
      {"SiBarrelRawHitAssociations", "SiBarrelVertexRawHitAssociations",
       "SiEndcapTrackerRawHitAssociations", "TOFBarrelRawHitAssociations",
       "TOFEndcapRawHitAssociations", "MPGDBarrelRawHitAssociations",
       "OuterMPGDBarrelRawHitAssociations", "BackwardMPGDEndcapRawHitAssociations",
       "ForwardMPGDEndcapRawHitAssociations", "B0TrackerRawHitAssociations"},
      {"CentralTrackingRawHitAssociations"}, // Output collection name
      app));

  app->Add(new JOmniFactoryGeneratorT<TrackerMeasurementFromHits_factory>(
      "CentralTrackerMeasurements", {"CentralTrackingRecHits"}, {"CentralTrackerMeasurements"},
      app));

  app->Add(new JOmniFactoryGeneratorT<CKFTracking_factory>(
      "CentralCKFTruthSeededTrajectories", {"CentralTrackTruthSeeds", "CentralTrackerMeasurements"},
      {
          "CentralCKFTruthSeededActsTrajectoriesUnfiltered",
          "CentralCKFTruthSeededActsTracksUnfiltered",
      },
      app));

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
      app));

  app->Add(new JOmniFactoryGeneratorT<AmbiguitySolver_factory>(
      "TruthSeededAmbiguityResolutionSolver",
      {"CentralCKFTruthSeededActsTracksUnfiltered", "CentralTrackerMeasurements"},
      {
          "CentralCKFTruthSeededActsTracks",
          "CentralCKFTruthSeededActsTrajectories",
      },
      app));

  app->Add(
      new JOmniFactoryGeneratorT<ActsToTracks_factory>("CentralCKFTruthSeededTracks",
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
                                                       app));

  app->Add(new JOmniFactoryGeneratorT<TrackSeeding_factory>(
      "CentralTrackSeedingResults", {"CentralTrackingRecHits"}, {"CentralTrackSeedingResults"}, {},
      app));

  app->Add(new JOmniFactoryGeneratorT<CKFTracking_factory>(
      "CentralCKFTrajectories", {"CentralTrackSeedingResults", "CentralTrackerMeasurements"},
      {
          "CentralCKFActsTrajectoriesUnfiltered",
          "CentralCKFActsTracksUnfiltered",
      },
      app));

  app->Add(
      new JOmniFactoryGeneratorT<ActsToTracks_factory>("CentralCKFTracksUnfiltered",
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
                                                       app));

  app->Add(new JOmniFactoryGeneratorT<AmbiguitySolver_factory>(
      "AmbiguityResolutionSolver", {"CentralCKFActsTracksUnfiltered", "CentralTrackerMeasurements"},
      {
          "CentralCKFActsTracks",
          "CentralCKFActsTrajectories",
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<ActsToTracks_factory>("CentralCKFTracks",
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
                                                            app));

  app->Add(new JOmniFactoryGeneratorT<TrackProjector_factory>("CentralTrackSegments",
                                                              {
                                                                  "CentralCKFActsTrajectories",
                                                                  "CentralCKFTracks",
                                                              },
                                                              {
                                                                  "CentralTrackSegments",
                                                              },
                                                              app));

  app->Add(new JOmniFactoryGeneratorT<IterativeVertexFinder_factory>(
      "CentralTrackVertices", {"CentralCKFActsTrajectories", "ReconstructedChargedParticles"},
      {"CentralTrackVertices"}, {}, app));

  app->Add(new JOmniFactoryGeneratorT<TrackPropagation_factory>(
      "CalorimeterTrackPropagator",
      {"CentralCKFTracks", "CentralCKFActsTrajectories", "CentralCKFActsTracks"},
      {"CalorimeterTrackProjections"},
      {.target_surfaces{
          // Ecal
          eicrecon::DiscSurfaceConfig{"EcalEndcapN_ID", "- EcalEndcapN_zmin", 0.,
                                      "1.1*EcalEndcapN_rmax"},
          eicrecon::DiscSurfaceConfig{"EcalEndcapN_ID", "- EcalEndcapN_zmin - 50*mm", 0.,
                                      "1.1*EcalEndcapN_rmax"},
          eicrecon::CylinderSurfaceConfig{
              "EcalBarrel_ID", "EcalBarrel_rmin",
              "- 1.1*max(EcalBarrelBackward_zmax,EcalBarrelForward_zmax)",
              "1.1*max(EcalBarrelBackward_zmax,EcalBarrelForward_zmax)"},
          eicrecon::CylinderSurfaceConfig{
              "EcalBarrel_ID", "EcalBarrel_rmin + 50*mm",
              "- 1.1*max(EcalBarrelBackward_zmax,EcalBarrelForward_zmax)",
              "1.1*max(EcalBarrelBackward_zmax,EcalBarrelForward_zmax)"},
          eicrecon::DiscSurfaceConfig{"EcalEndcapP_ID", "EcalEndcapP_zmin", 0.,
                                      "1.1*EcalEndcapP_rmax"},
          eicrecon::DiscSurfaceConfig{"EcalEndcapP_ID", "EcalEndcapP_zmin + 50*mm", 0.,
                                      "1.1*EcalEndcapP_rmax"},
          // Hcal
          eicrecon::DiscSurfaceConfig{"HcalEndcapN_ID", "- HcalEndcapN_zmin", 0.,
                                      "1.1*HcalEndcapN_rmax"},
          eicrecon::DiscSurfaceConfig{"HcalEndcapN_ID", "- HcalEndcapN_zmin - 150*mm", 0.,
                                      "1.1*HcalEndcapN_rmax"},
          eicrecon::CylinderSurfaceConfig{
              "HcalBarrel_ID", "HcalBarrel_rmin",
              "- 1.1*max(HcalBarrelBackward_zmax,HcalBarrelForward_zmax)",
              "1.1*max(HcalBarrelBackward_zmax,HcalBarrelForward_zmax)"},
          eicrecon::CylinderSurfaceConfig{
              "HcalBarrel_ID", "HcalBarrel_rmin + 150*mm",
              "- 1.1*max(HcalBarrelBackward_zmax,HcalBarrelForward_zmax)",
              "1.1*max(HcalBarrelBackward_zmax,HcalBarrelForward_zmax)"},
          eicrecon::DiscSurfaceConfig{"LFHCAL_ID", "LFHCAL_zmin", 0., "1.1*LFHCAL_rmax"},
          eicrecon::DiscSurfaceConfig{"LFHCAL_ID", "LFHCAL_zmin + 150*mm", 0., "1.1*LFHCAL_rmax"},
      }},
      app));

  // Add central and other tracks
  app->Add(new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::Track, true>>(
      "CombinedTracks", {"CentralCKFTracks", "TaggerTrackerTracks"}, {"CombinedTracks"}, app));

    app->Add(new JOmniFactoryGeneratorT<SecondaryVertexFinder_factory>(
            "AMVSecondaryTrackVertices",{"ReconstructedParticles","CentralCKFActsTrajectories"},
            {"AMVPrimaryVertices","AMVSecondaryTrackVertices",},{},app));

  app->Add(new JOmniFactoryGeneratorT<
           CollectionCollector_factory<edm4eic::MCRecoTrackParticleAssociation, true>>(
      "CombinedTrackAssociations",
      {"CentralCKFTrackAssociations", "TaggerTrackerTrackAssociations"},
      {"CombinedTrackAssociations"}, app));

  app->Add(new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::Track, true>>(
      "CombinedTruthSeededTracks", {"CentralCKFTruthSeededTracks", "TaggerTrackerTracks"},
      {"CombinedTruthSeededTracks"}, app));

  app->Add(new JOmniFactoryGeneratorT<
           CollectionCollector_factory<edm4eic::MCRecoTrackParticleAssociation, true>>(
      "CombinedTruthSeededTrackAssociations",
      {"CentralCKFTruthSeededTrackAssociations", "TaggerTrackerTrackAssociations"},
      {"CombinedTruthSeededTrackAssociations"}, app));

  app->Add(new JOmniFactoryGeneratorT<TracksToParticles_factory>(
      "ChargedTruthSeededParticlesWithAssociations",
      {
          "CombinedTruthSeededTracks",
          "CombinedTruthSeededTrackAssociations",
      },
      {"ReconstructedTruthSeededChargedWithoutPIDParticles",
       "ReconstructedTruthSeededChargedWithoutPIDParticleAssociations"},
      {}, app));

  app->Add(new JOmniFactoryGeneratorT<TracksToParticles_factory>(
      "ChargedParticlesWithAssociations",
      {
          "CombinedTracks",
          "CombinedTrackAssociations",
      },
      {"ReconstructedChargedWithoutPIDParticles",
       "ReconstructedChargedWithoutPIDParticleAssociations"},
      {}, app));
}
} // extern "C"
