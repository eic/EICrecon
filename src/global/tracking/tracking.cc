// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025, Dmitry Romanov, Tyler Kutz, Wouter Deconinck, Dmitry Kalinkin

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/MCRecoTrackParticleAssociationCollection.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
#include <edm4eic/MCRecoTrackerHitLinkCollection.h>
#endif
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackParameters.h>
#include <edm4eic/TrackSeed.h>
#include <edm4eic/TrackerHitCollection.h>
#include <podio/detail/Link.h>
#include <deque>
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
#include "factories/tracking/ActsTrackMerger_factory.h"
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
      "TrackTruthSeeds", {"EventHeader", "MCParticles"},
      {"TrackTruthSeeds", "TrackTruthSeedParameters"}, {}, app));

  std::vector<std::pair<double, double>> thetaRanges{{0, 50 * dd4hep::mrad},
                                                     {50 * dd4hep::mrad, 180 * dd4hep::deg}};
  app->Add(new JOmniFactoryGeneratorT<SubDivideCollection_factory<edm4eic::TrackSeed>>(
      "CentralB0TrackTruthSeeds", {"TrackTruthSeeds"},
      {"B0TrackerTruthSeeds", "CentralTrackerTruthSeeds"},
      {
          .function = RangeSplit<
              Chain<&edm4eic::TrackSeed::getParams, &edm4eic::TrackParameters::getTheta>>(
              thetaRanges),
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

#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  // Tracker hit links collector
  app->Add(
      new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::MCRecoTrackerHitLink, true>>(
          "CentralTrackingRawHitLinks",
          {"SiBarrelRawHitLinks", "SiBarrelVertexRawHitLinks", "SiEndcapTrackerRawHitLinks",
           "TOFBarrelRawHitLinks", "TOFEndcapRawHitLinks", "MPGDBarrelRawHitLinks",
           "OuterMPGDBarrelRawHitLinks", "BackwardMPGDEndcapRawHitLinks",
           "ForwardMPGDEndcapRawHitLinks"},
          {"CentralTrackingRawHitLinks"}, // Output collection name
          app));
#endif

  app->Add(new JOmniFactoryGeneratorT<TrackerMeasurementFromHits_factory>(
      "CentralTrackerMeasurements", {"CentralTrackingRecHits"}, {"CentralTrackerMeasurements"},
      app));

  app->Add(new JOmniFactoryGeneratorT<CKFTracking_factory>(
      "CentralCKFTruthSeededTrajectories",
      {"CentralTrackerTruthSeeds", "CentralTrackerMeasurements"},
      {
          "CentralCKFTruthSeededActsTrackStatesUnfiltered",
          "CentralCKFTruthSeededActsTracksUnfiltered",
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<ActsToTracks_factory>(
      "CentralCKFTruthSeededTracksUnfiltered",
      {
          "CentralTrackerMeasurements",
          "CentralTrackerTruthSeeds",
          "CentralCKFTruthSeededActsTrackStatesUnfiltered",
          "CentralCKFTruthSeededActsTracksUnfiltered",
          "CentralTrackingRawHitAssociations",
      },
      {
          "CentralCKFTruthSeededTrajectoriesUnfiltered",
          "CentralCKFTruthSeededTrackParametersUnfiltered",
          "CentralCKFTruthSeededTracksUnfiltered",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
          "CentralCKFTruthSeededTrackUnfilteredLinks",
#endif
          "CentralCKFTruthSeededTrackUnfilteredAssociations",
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<AmbiguitySolver_factory>(
      "TruthSeededAmbiguityResolutionSolver",
      {"CentralCKFTruthSeededActsTrackStatesUnfiltered",
       "CentralCKFTruthSeededActsTracksUnfiltered"},
      {
          "CentralCKFTruthSeededActsTrackStates",
          "CentralCKFTruthSeededActsTracks",
      },
      app));

  app->Add(
      new JOmniFactoryGeneratorT<ActsToTracks_factory>("CentralCKFTruthSeededTracks",
                                                       {
                                                           "CentralTrackerMeasurements",
                                                           "CentralTrackerTruthSeeds",
                                                           "CentralCKFTruthSeededActsTrackStates",
                                                           "CentralCKFTruthSeededActsTracks",
                                                           "CentralTrackingRawHitAssociations",
                                                       },
                                                       {
                                                           "CentralCKFTruthSeededTrajectories",
                                                           "CentralCKFTruthSeededTrackParameters",
                                                           "CentralCKFTruthSeededTracks",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                                           "CentralCKFTruthSeededTrackLinks",
#endif
                                                           "CentralCKFTruthSeededTrackAssociations",
                                                       },
                                                       app));

  app->Add(new JOmniFactoryGeneratorT<TrackSeeding_factory>(
      "CentralTrackSeeds", {"CentralTrackingRecHits"},
      {"CentralTrackSeeds", "CentralTrackSeedParameters"}, {}, app));

  app->Add(new JOmniFactoryGeneratorT<CKFTracking_factory>(
      "CentralCKFTrajectories", {"CentralTrackSeeds", "CentralTrackerMeasurements"},
      {
          "CentralCKFActsTrackStatesUnfiltered",
          "CentralCKFActsTracksUnfiltered",
      },
      app));

  app->Add(
      new JOmniFactoryGeneratorT<ActsToTracks_factory>("CentralCKFTracksUnfiltered",
                                                       {
                                                           "CentralTrackerMeasurements",
                                                           "CentralTrackSeeds",
                                                           "CentralCKFActsTrackStatesUnfiltered",
                                                           "CentralCKFActsTracksUnfiltered",
                                                           "CentralTrackingRawHitAssociations",
                                                       },
                                                       {
                                                           "CentralCKFTrajectoriesUnfiltered",
                                                           "CentralCKFTrackParametersUnfiltered",
                                                           "CentralCKFTracksUnfiltered",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                                           "CentralCKFTrackUnfilteredLinks",
#endif
                                                           "CentralCKFTrackUnfilteredAssociations",
                                                       },
                                                       app));

  app->Add(new JOmniFactoryGeneratorT<AmbiguitySolver_factory>(
      "AmbiguityResolutionSolver",
      {"CentralCKFActsTrackStatesUnfiltered", "CentralCKFActsTracksUnfiltered"},
      {
          "CentralCKFActsTrackStates",
          "CentralCKFActsTracks",
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<ActsToTracks_factory>("CentralCKFTracks",
                                                            {
                                                                "CentralTrackerMeasurements",
                                                                "CentralTrackSeeds",
                                                                "CentralCKFActsTrackStates",
                                                                "CentralCKFActsTracks",
                                                                "CentralTrackingRawHitAssociations",
                                                            },
                                                            {
                                                                "CentralCKFTrajectories",
                                                                "CentralCKFTrackParameters",
                                                                "CentralCKFTracks",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                                                "CentralCKFTrackLinks",
#endif
                                                                "CentralCKFTrackAssociations",
                                                            },
                                                            app));

  app->Add(new JOmniFactoryGeneratorT<TrackProjector_factory>("CentralTrackSegments",
                                                              {
                                                                  "CentralCKFActsTrackStates",
                                                                  "CentralCKFActsTracks",
                                                                  "CentralCKFTracks",
                                                              },
                                                              {
                                                                  "CentralTrackSegments",
                                                              },
                                                              app));

  app->Add(
      new JOmniFactoryGeneratorT<IterativeVertexFinder_factory>("CentralTrackVertices",
                                                                {
                                                                    "CentralCKFActsTrackStates",
                                                                    "CentralCKFActsTracks",
                                                                    "ReconstructedChargedParticles",
                                                                },
                                                                {
                                                                    "CentralTrackVertices",
                                                                },
                                                                {}, app));

  app->Add(new JOmniFactoryGeneratorT<TrackPropagation_factory>(
      "CalorimeterTrackPropagator",
      {"CentralCKFTracks", "CentralCKFActsTrackStates", "CentralCKFActsTracks"},
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
          "B0TrackerCKFTruthSeededActsTrackStatesUnfiltered",
          "B0TrackerCKFTruthSeededActsTracksUnfiltered",
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<ActsToTracks_factory>(
      "B0TrackerCKFTruthSeededTracksUnfiltered",
      {
          "B0TrackerMeasurements",
          "B0TrackerTruthSeeds",
          "B0TrackerCKFTruthSeededActsTrackStatesUnfiltered",
          "B0TrackerCKFTruthSeededActsTracksUnfiltered",
          "B0TrackerRawHitAssociations",
      },
      {
          "B0TrackerCKFTruthSeededTrajectoriesUnfiltered",
          "B0TrackerCKFTruthSeededTrackParametersUnfiltered",
          "B0TrackerCKFTruthSeededTracksUnfiltered",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
          "B0TrackerCKFTruthSeededTrackUnfilteredLinks",
#endif
          "B0TrackerCKFTruthSeededTrackUnfilteredAssociations",
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<AmbiguitySolver_factory>(
      "B0TrackerTruthSeededAmbiguityResolutionSolver",
      {"B0TrackerCKFTruthSeededActsTrackStatesUnfiltered",
       "B0TrackerCKFTruthSeededActsTracksUnfiltered"},
      {
          "B0TrackerCKFTruthSeededActsTrackStates",
          "B0TrackerCKFTruthSeededActsTracks",
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<ActsToTracks_factory>(
      "B0TrackerCKFTruthSeededTracks",
      {
          "B0TrackerMeasurements",
          "B0TrackerTruthSeeds",
          "B0TrackerCKFTruthSeededActsTrackStates",
          "B0TrackerCKFTruthSeededActsTracks",
          "B0TrackerRawHitAssociations",
      },
      {
          "B0TrackerCKFTruthSeededTrajectories",
          "B0TrackerCKFTruthSeededTrackParameters",
          "B0TrackerCKFTruthSeededTracks",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
          "B0TrackerCKFTruthSeededTrackLinks",
#endif
          "B0TrackerCKFTruthSeededTrackAssociations",
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<TrackSeeding_factory>(
      "B0TrackerSeeds", {"B0TrackerRecHits"}, {"B0TrackerSeeds", "B0TrackerSeedParameters"}, {},
      app));

  app->Add(new JOmniFactoryGeneratorT<CKFTracking_factory>(
      "B0TrackerCKFTrajectories", {"B0TrackerSeeds", "B0TrackerMeasurements"},
      {
          "B0TrackerCKFActsTrackStatesUnfiltered",
          "B0TrackerCKFActsTracksUnfiltered",
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<ActsToTracks_factory>(
      "B0TrackerCKFTracksUnfiltered",
      {
          "B0TrackerMeasurements",
          "B0TrackerSeeds",
          "B0TrackerCKFActsTrackStatesUnfiltered",
          "B0TrackerCKFActsTracksUnfiltered",
          "B0TrackerRawHitAssociations",
      },
      {
          "B0TrackerCKFTrajectoriesUnfiltered",
          "B0TrackerCKFTrackParametersUnfiltered",
          "B0TrackerCKFTracksUnfiltered",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
          "B0TrackerCKFTrackUnfilteredLinks",
#endif
          "B0TrackerCKFTrackUnfilteredAssociations",
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<AmbiguitySolver_factory>(
      "B0TrackerAmbiguityResolutionSolver",
      {"B0TrackerCKFActsTrackStatesUnfiltered", "B0TrackerCKFActsTracksUnfiltered"},
      {
          "B0TrackerCKFActsTrackStates",
          "B0TrackerCKFActsTracks",
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<ActsToTracks_factory>("B0TrackerCKFTracks",
                                                            {
                                                                "B0TrackerMeasurements",
                                                                "B0TrackerSeeds",
                                                                "B0TrackerCKFActsTrackStates",
                                                                "B0TrackerCKFActsTracks",
                                                                "B0TrackerRawHitAssociations",
                                                            },
                                                            {
                                                                "B0TrackerCKFTrajectories",
                                                                "B0TrackerCKFTrackParameters",
                                                                "B0TrackerCKFTracks",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                                                "B0TrackerCKFTrackLinks",
#endif
                                                                "B0TrackerCKFTrackAssociations",
                                                            },
                                                            app));

  // COMBINED TRACKING

  // Use both central and B0 tracks for vertexing
  app->Add(new JOmniFactoryGeneratorT<ActsTrackMerger_factory>(
      "CentralAndB0TrackerCKFActsTracks",
      {
          "CentralCKFActsTrackStates",
          "CentralCKFActsTracks",
          "B0TrackerCKFActsTrackStates",
          "B0TrackerCKFActsTracks",
      },
      {
          "CentralAndB0TrackerCKFActsTrackStates",
          "CentralAndB0TrackerCKFActsTracks",
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<IterativeVertexFinder_factory>(
      "CentralAndB0TrackVertices",
      {
          "CentralAndB0TrackerCKFActsTrackStates",
          "CentralAndB0TrackerCKFActsTracks",
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
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "ReconstructedTruthSeededChargedWithoutPIDParticleLinks",
#endif
       "ReconstructedTruthSeededChargedWithoutPIDParticleAssociations"},
      {}, app));

  app->Add(new JOmniFactoryGeneratorT<TracksToParticles_factory>(
      "ChargedParticlesWithAssociations",
      {
          "CombinedTracks",
          "CombinedTrackAssociations",
      },
      {"ReconstructedChargedWithoutPIDParticles",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "ReconstructedChargedWithoutPIDParticleLinks",
#endif
       "ReconstructedChargedWithoutPIDParticleAssociations"},
      {}, app));
}
} // extern "C"
