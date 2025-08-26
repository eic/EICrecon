// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson

#include <edm4eic/EDM4eicVersion.h>
#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <memory>

#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/particle/TrackClusterSubtractor_factory.h"

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

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "EcalEndcapNSubtractedClusters",
      {"EcalEndcapNTrackClusterMatches", "CalorimeterTrackProjections"},
      {"EcalEndcapNSubtractedClusters", "EcalEndcapNRemnantClusters",
       "EcalEndcapNTrackSubtractedClusterMatches"},
      {
          .fracEnergyToSub = 1.0,
          .defaultMassPdg  = 211,
          .surfaceToUse    = 1,
      },
      app // TODO: remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "HcalEndcapNSubtractedClusters",
      {"HcalEndcapNTrackClusterMatches", "CalorimeterTrackProjections"},
      {"HcalEndcapNSubtractedClusters", "HcalEndcapNRemnantClusters",
       "HcalEndcapNTrackSubtractedClusterMatches"},
      {.fracEnergyToSub = 1.0, .defaultMassPdg = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  // central ------------------------------------------------------------

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "EcalBarrelSubtractedClusters",
      {"EcalBarrelTrackClusterMatches", "CalorimeterTrackProjections"},
      {"EcalBarrelSubtractedClusters", "EcalBarrelRemnantClusters",
       "EcalBarrelTrackSubtractedClusterMatches"},
      {.fracEnergyToSub = 1.0, .defaultMassPdg = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "HcalBarrelSubtractedClusters",
      {"HcalBarrelTrackClusterMatches", "CalorimeterTrackProjections"},
      {"HcalBarrelSubtractedClusters", "HcalBarrelRemnantClusters",
       "HcalBarrelTrackSubtractedClusterMatches"},
      {.fracEnergyToSub = 1.0, .defaultMassPdg = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  // forward ------------------------------------------------------------

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "EcalEndcapPSubtractedClusters",
      {"EcalEndcapPTrackClusterMatches", "CalorimeterTrackProjections"},
      {"EcalEndcapPSubtractedClusters", "EcalEndcapPRemnantClusters",
       "EcalEndcapPTrackSubtractedClusterMatches"},
      {.fracEnergyToSub = 1.0, .defaultMassPdg = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "LFHCALSubtractedClusters",
      {"LFHCALTrackSplitMergeClusterMatches", "CalorimeterTrackProjections"},
      {"LFHCALSubtractedClusters", "LFHCALRemnantClusters", "LFHCALTrackSubtractedClusterMatches"},
      {.fracEnergyToSub = 1.0, .defaultMassPdg = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

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
