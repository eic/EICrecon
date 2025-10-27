// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025, Dmitry Romanov, Tyler Kutz, Wouter Deconinck, Dmitry Kalinkin

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <edm4eic/MCRecoTrackParticleAssociationCollection.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackParameters.h>
#include <edm4eic/TrackerHitCollection.h>
#include <fmt/core.h>
#include <cmath>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "algorithms/meta/SubDivideFunctors.h"
#include "algorithms/tracking/TrackPropagationConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/meta/CollectionCollector_factory.h"
#include "factories/meta/SubDivideCollection_factory.h"
#include "factories/tracking/ActsToTracks_factory.h"
#include "factories/tracking/ActsTrajectoriesMerger_factory.h"
#include "factories/tracking/AmbiguitySolver_factory.h"
#include "factories/tracking/CKFTracking_factory.h"
#include "factories/tracking/IterativeVertexFinder_factory.h"
#include "factories/tracking/TrackParamTruthInit_factory.h"
#include "factories/tracking/TrackProjector_factory.h"
#include "factories/tracking/TrackPropagation_factory.h"
#include "factories/tracking/TrackSeeding_factory.h"
#include "factories/tracking/TrackerMeasurementFromHits_factory.h"
#include "factories/tracking/TracksToParticles_factory.h"

//
extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);

  using namespace eicrecon;

  app->Add(new JOmniFactoryGeneratorT<TrackParamTruthInit_factory>(
      "TrackTruthSeeds", {"EventHeader", "MCParticles"}, {"TrackTruthSeeds"}, {}, app));

  std::vector<std::pair<double, double>> thetaRanges{{0, 50 * dd4hep::mrad},
                                                     {50 * dd4hep::mrad, 180 * dd4hep::deg}};
  app->Add(new JOmniFactoryGeneratorT<SubDivideCollection_factory<edm4eic::TrackParameters>>(
      "CentralB0TrackTruthSeeds", {"TrackTruthSeeds"},
      {"B0TrackerTruthSeeds", "CentralTrackerTruthSeeds"},
      {
          .function = RangeSplit<&edm4eic::TrackParameters::getTheta>(thetaRanges),
      },
      app));

  // CENTRAL TRACKER

  // Tracker hits collector
  app->Add(new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::TrackerHit, true>>(
      "CentralTrackingRecHits",
      {"SiBarrelTrackerRecHits", "SiBarrelVertexRecHits", "SiEndcapTrackerRecHits",
       "TOFBarrelRecHits", "TOFEndcapRecHits", "MPGDBarrelRecHits", "OuterMPGDBarrelRecHits",
       "BackwardMPGDEndcapRecHits", "ForwardMPGDEndcapRecHits"},
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
       "ForwardMPGDEndcapRawHitAssociations"},
      {"CentralTrackingRawHitAssociations"}, // Output collection name
      app));

  app->Add(new JOmniFactoryGeneratorT<TrackerMeasurementFromHits_factory>(
      "CentralTrackerMeasurements", {"CentralTrackingRecHits"}, {"CentralTrackerMeasurements"},
      app));

  app->Add(new JOmniFactoryGeneratorT<CKFTracking_factory>(
      "CentralCKFTruthSeededTrajectories",
      {"CentralTrackerTruthSeeds", "CentralTrackerMeasurements"},
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

  app->Add(
      new JOmniFactoryGeneratorT<IterativeVertexFinder_factory>("CentralTrackVertices",
                                                                {
                                                                    "CentralCKFActsTrajectories",
                                                                    "ReconstructedChargedParticles",
                                                                },
                                                                {
                                                                    "CentralTrackVertices",
                                                                },
                                                                {}, app));

  app->Add(new JOmniFactoryGeneratorT<TrackPropagation_factory>(
      "CalorimeterTrackPropagator",
      {"CentralCKFTracks", "CentralCKFActsTrajectories", "CentralCKFActsTracks"},
      {"CalorimeterTrackProjections"},
      {.target_surfaces{
          // Ecal
          eicrecon::DiscSurfaceConfig{.id   = "EcalEndcapN_ID",
                                      .zmin = "- EcalEndcapN_zmin",
                                      .rmin = 0.,
                                      .rmax = "1.1*EcalEndcapN_rmax"},
          eicrecon::DiscSurfaceConfig{.id   = "EcalEndcapN_ID",
                                      .zmin = "- EcalEndcapN_zmin - 50*mm",
                                      .rmin = 0.,
                                      .rmax = "1.1*EcalEndcapN_rmax"},
          eicrecon::CylinderSurfaceConfig{
              .id   = "EcalBarrel_ID",
              .rmin = "EcalBarrel_rmin",
              .zmin = "- 1.1*max(EcalBarrelBackward_zmax,EcalBarrelForward_zmax)",
              .zmax = "1.1*max(EcalBarrelBackward_zmax,EcalBarrelForward_zmax)"},
          eicrecon::CylinderSurfaceConfig{
              .id   = "EcalBarrel_ID",
              .rmin = "EcalBarrel_rmin + 50*mm",
              .zmin = "- 1.1*max(EcalBarrelBackward_zmax,EcalBarrelForward_zmax)",
              .zmax = "1.1*max(EcalBarrelBackward_zmax,EcalBarrelForward_zmax)"},
          eicrecon::DiscSurfaceConfig{.id   = "EcalEndcapP_ID",
                                      .zmin = "EcalEndcapP_zmin",
                                      .rmin = 0.,
                                      .rmax = "1.1*EcalEndcapP_rmax"},
          eicrecon::DiscSurfaceConfig{.id   = "EcalEndcapP_ID",
                                      .zmin = "EcalEndcapP_zmin + 50*mm",
                                      .rmin = 0.,
                                      .rmax = "1.1*EcalEndcapP_rmax"},
          // Hcal
          eicrecon::DiscSurfaceConfig{.id   = "HcalEndcapN_ID",
                                      .zmin = "- HcalEndcapN_zmin",
                                      .rmin = 0.,
                                      .rmax = "1.1*HcalEndcapN_rmax"},
          eicrecon::DiscSurfaceConfig{.id   = "HcalEndcapN_ID",
                                      .zmin = "- HcalEndcapN_zmin - 150*mm",
                                      .rmin = 0.,
                                      .rmax = "1.1*HcalEndcapN_rmax"},
          eicrecon::CylinderSurfaceConfig{
              .id   = "HcalBarrel_ID",
              .rmin = "HcalBarrel_rmin",
              .zmin = "- 1.1*max(HcalBarrelBackward_zmax,HcalBarrelForward_zmax)",
              .zmax = "1.1*max(HcalBarrelBackward_zmax,HcalBarrelForward_zmax)"},
          eicrecon::CylinderSurfaceConfig{
              .id   = "HcalBarrel_ID",
              .rmin = "HcalBarrel_rmin + 150*mm",
              .zmin = "- 1.1*max(HcalBarrelBackward_zmax,HcalBarrelForward_zmax)",
              .zmax = "1.1*max(HcalBarrelBackward_zmax,HcalBarrelForward_zmax)"},
          eicrecon::DiscSurfaceConfig{
              .id = "LFHCAL_ID", .zmin = "LFHCAL_zmin", .rmin = 0., .rmax = "1.1*LFHCAL_rmax"},
          eicrecon::DiscSurfaceConfig{.id   = "LFHCAL_ID",
                                      .zmin = "LFHCAL_zmin + 150*mm",
                                      .rmin = 0.,
                                      .rmax = "1.1*LFHCAL_rmax"},
      }},
      app));

  // B0 TRACKER

  app->Add(new JOmniFactoryGeneratorT<TrackerMeasurementFromHits_factory>(
      "B0TrackerMeasurements", {"B0TrackerRecHits"}, {"B0TrackerMeasurements"}, app));

  app->Add(new JOmniFactoryGeneratorT<CKFTracking_factory>(
      "B0TrackerCKFTruthSeededTrajectories", {"B0TrackerTruthSeeds", "B0TrackerMeasurements"},
      {
          "B0TrackerCKFTruthSeededActsTrajectoriesUnfiltered",
          "B0TrackerCKFTruthSeededActsTracksUnfiltered",
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<ActsToTracks_factory>(
      "B0TrackerCKFTruthSeededTracksUnfiltered",
      {
          "B0TrackerMeasurements",
          "B0TrackerCKFTruthSeededActsTrajectoriesUnfiltered",
          "B0TrackerRawHitAssociations",
      },
      {
          "B0TrackerCKFTruthSeededTrajectoriesUnfiltered",
          "B0TrackerCKFTruthSeededTrackParametersUnfiltered",
          "B0TrackerCKFTruthSeededTracksUnfiltered",
          "B0TrackerCKFTruthSeededTrackUnfilteredAssociations",
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<AmbiguitySolver_factory>(
      "B0TrackerTruthSeededAmbiguityResolutionSolver",
      {"B0TrackerCKFTruthSeededActsTracksUnfiltered", "B0TrackerMeasurements"},
      {
          "B0TrackerCKFTruthSeededActsTracks",
          "B0TrackerCKFTruthSeededActsTrajectories",
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<ActsToTracks_factory>(
      "B0TrackerCKFTruthSeededTracks",
      {
          "B0TrackerMeasurements",
          "B0TrackerCKFTruthSeededActsTrajectories",
          "B0TrackerRawHitAssociations",
      },
      {
          "B0TrackerCKFTruthSeededTrajectories",
          "B0TrackerCKFTruthSeededTrackParameters",
          "B0TrackerCKFTruthSeededTracks",
          "B0TrackerCKFTruthSeededTrackAssociations",
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<TrackSeeding_factory>(
      "B0TrackerTrackSeedingResults", {"B0TrackerRecHits"}, {"B0TrackerSeedingResults"}, {}, app));

  app->Add(new JOmniFactoryGeneratorT<CKFTracking_factory>(
      "B0TrackerCKFTrajectories", {"B0TrackerSeedingResults", "B0TrackerMeasurements"},
      {
          "B0TrackerCKFActsTrajectoriesUnfiltered",
          "B0TrackerCKFActsTracksUnfiltered",
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<ActsToTracks_factory>(
      "B0TrackerCKFTracksUnfiltered",
      {
          "B0TrackerMeasurements",
          "B0TrackerCKFActsTrajectoriesUnfiltered",
          "B0TrackerRawHitAssociations",
      },
      {
          "B0TrackerCKFTrajectoriesUnfiltered",
          "B0TrackerCKFTrackParametersUnfiltered",
          "B0TrackerCKFTracksUnfiltered",
          "B0TrackerCKFTrackUnfilteredAssociations",
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<AmbiguitySolver_factory>(
      "B0TrackerAmbiguityResolutionSolver",
      {"B0TrackerCKFActsTracksUnfiltered", "B0TrackerMeasurements"},
      {
          "B0TrackerCKFActsTracks",
          "B0TrackerCKFActsTrajectories",
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<ActsToTracks_factory>("B0TrackerCKFTracks",
                                                            {
                                                                "B0TrackerMeasurements",
                                                                "B0TrackerCKFActsTrajectories",
                                                                "B0TrackerRawHitAssociations",
                                                            },
                                                            {
                                                                "B0TrackerCKFTrajectories",
                                                                "B0TrackerCKFTrackParameters",
                                                                "B0TrackerCKFTracks",
                                                                "B0TrackerCKFTrackAssociations",
                                                            },
                                                            app));

  // COMBINED TRACKING

  // Use both central and B0 tracks for vertexing
  app->Add(new JOmniFactoryGeneratorT<ActsTrajectoriesMerger_factory>(
      "CentralB0CKFActsTrajectories",
      {
          "CentralCKFActsTrajectories",
          "B0TrackerCKFActsTrajectories",
      },
      {
          "CentralAndB0TrackerCKFActsTrajectories",
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<IterativeVertexFinder_factory>(
      "CentralAndB0TrackVertices",
      {
          "CentralAndB0TrackerCKFActsTrajectories",
          "ReconstructedChargedParticles",
      },
      {
          "CentralAndB0TrackVertices",
      },
      {}, app));

  // Add central and B0 tracks
  app->Add(new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::Track, true>>(
      "CombinedTracks", {"CentralCKFTracks", "B0TrackerCKFTracks"}, {"CombinedTracks"}, app));

  app->Add(new JOmniFactoryGeneratorT<
           CollectionCollector_factory<edm4eic::MCRecoTrackParticleAssociation, true>>(
      "CombinedTrackAssociations", {"CentralCKFTrackAssociations", "B0TrackerCKFTrackAssociations"},
      {"CombinedTrackAssociations"}, app));

  app->Add(new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::Track, true>>(
      "CombinedTruthSeededTracks", {"CentralCKFTruthSeededTracks", "B0TrackerCKFTruthSeededTracks"},
      {"CombinedTruthSeededTracks"}, app));

  app->Add(new JOmniFactoryGeneratorT<
           CollectionCollector_factory<edm4eic::MCRecoTrackParticleAssociation, true>>(
      "CombinedTruthSeededTrackAssociations",
      {"CentralCKFTruthSeededTrackAssociations", "B0TrackerCKFTruthSeededTrackAssociations"},
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
