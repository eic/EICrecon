// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson

#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <edm4eic/EDM4eicVersion.h>
#include <string>
#include <vector>

#include "extensions/jana/JOmniFactoryGeneratorT.h"

#if EDM4EIC_VERSION_MAJOR >= 8
#include "factories/particle/TrackClusterSubtractor_factory.h"
#endif

extern "C" {

void InitPlugin(JApplication* app) {

  using namespace eicrecon;

  InitJANAPlugin(app);

  // ====================================================================
  // PFAlpha: baseline PF implementation
  // ====================================================================

  // --------------------------------------------------------------------
  // PFA (0) connection: split/merge clusters accordingly
  // --------------------------------------------------------------------

  /* TODO move here when ready */

  // --------------------------------------------------------------------
  // PFA (1a) arbitration: apply track correction to clusters
  // --------------------------------------------------------------------

  // backward -----------------------------------------------------------

#if EDM4EIC_VERSION_MAJOR >= 8
  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "EcalEndcapNSubtractedClusters",
      {"EcalEndcapNTrackClusterMatches", "EcalEndcapNClusters", "CalorimeterTrackProjections"},
      {"EcalEndcapNRemnantClusters", "EcalEndcapNExpectedClusters",
       "EcalEndcapNTrackExpectedClusterMatches"},
      {.fracEnergyToSub = 1.0, .defaultMassPdg = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "HcalEndcapNSubtractedClusters",
      {"HcalEndcapNTrackClusterMatches", "HcalEndcapNClusters", "CalorimeterTrackProjections"},
      {"HcalEndcapNRemnantClusters", "HcalEndcapNExpectedClusters",
       "HcalEndcapNTrackExpectedClusterMatches"},
      {.fracEnergyToSub = 1.0, .defaultMassPdg = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  // central ------------------------------------------------------------

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "EcalBarrelSubtractedClusters",
      {"EcalBarrelTrackClusterMatches", "EcalBarrelClusters", "CalorimeterTrackProjections"},
      {"EcalBarrelRemnantClusters", "EcalBarrelExpectedClusters",
       "EcalBarrelTrackExpectedClusterMatches"},
      {.fracEnergyToSub = 1.0, .defaultMassPdg = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "HcalBarrelSubtractedClusters",
      {"HcalBarrelTrackClusterMatches", "HcalBarrelClusters", "CalorimeterTrackProjections"},
      {"HcalBarrelRemnantClusters", "HcalBarrelExpectedClusters",
       "HcalBarrelTrackExpectedClusterMatches"},
      {.fracEnergyToSub = 1.0, .defaultMassPdg = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  // forward ------------------------------------------------------------

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "EcalEndcapPSubtractedClusters",
      {"EcalEndcapPTrackClusterMatches", "EcalEndcapPClusters", "CalorimeterTrackProjections"},
      {"EcalEndcapPRemnantClusters", "EcalEndcapPExpectedClusters",
       "EcalEndcapPTrackExpectedClusterMatches"},
      {.fracEnergyToSub = 1.0, .defaultMassPdg = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "LFHCALSubtractedClusters",
      {"LFHCALTrackSplitMergeClusterMatches", "LFHCALClusters", "CalorimeterTrackProjections"},
      {"LFHCALRemnantClusters", "LFHCALExpectedClusters", "LFHCALTrackExpectedClusterMatches"},
      {.fracEnergyToSub = 1.0, .defaultMassPdg = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));
#endif

  // --------------------------------------------------------------------
  // PFA (1b) arbitration: form charged candidates
  // --------------------------------------------------------------------

  /* TODO add here */

  // --------------------------------------------------------------------
  // PFA (2) arbitration: combine remnants, form neutral candidates
  // --------------------------------------------------------------------

  /* TODO add here */

  // --------------------------------------------------------------------
  // PFA (3) regression: convert candidates to reco particles
  // --------------------------------------------------------------------

  /* TODO add here */
}
} // extern "C"
