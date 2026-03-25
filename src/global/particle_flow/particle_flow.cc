// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson

#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <string>
#include <vector>

#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/particle_flow/TrackClusterSubtractor_factory.h"

extern "C" {

void InitPlugin(JApplication* app) {

  using namespace eicrecon;

  InitJANAPlugin(app);

  // ====================================================================
  // PFAlpha: baseline PF implementation
  // ====================================================================

  // backward -----------------------------------------------------------

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "EcalEndcapNRemnantClusters",
      {"EcalEndcapNTrackClusterMatches", "EcalEndcapNClusters", "CalorimeterTrackProjections"},
      {"EcalEndcapNRemnantClusters", "EcalEndcapNExpectedClusters",
       "EcalEndcapNTrackExpectedClusterMatches"},
      {.energyFractionToSubtract = 1.0, .defaultPDG = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "HcalEndcapNRemnantClusters",
      {"HcalEndcapNTrackClusterMatches", "HcalEndcapNClusters", "CalorimeterTrackProjections"},
      {"HcalEndcapNRemnantClusters", "HcalEndcapNExpectedClusters",
       "HcalEndcapNTrackExpectedClusterMatches"},
      {.energyFractionToSubtract = 1.0, .defaultPDG = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  // central ------------------------------------------------------------

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "EcalBarrelRemnantClusters",
      {"EcalBarrelTrackClusterMatches", "EcalBarrelClusters", "CalorimeterTrackProjections"},
      {"EcalBarrelRemnantClusters", "EcalBarrelExpectedClusters",
       "EcalBarrelTrackExpectedClusterMatches"},
      {.energyFractionToSubtract = 1.0, .defaultPDG = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "HcalBarrelRemnantClusters",
      {"HcalBarrelTrackClusterMatches", "HcalBarrelClusters", "CalorimeterTrackProjections"},
      {"HcalBarrelRemnantClusters", "HcalBarrelExpectedClusters",
       "HcalBarrelTrackExpectedClusterMatches"},
      {.energyFractionToSubtract = 1.0, .defaultPDG = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  // forward ------------------------------------------------------------

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "EcalEndcapPRemnantClusters",
      {"EcalEndcapPTrackClusterMatches", "EcalEndcapPClusters", "CalorimeterTrackProjections"},
      {"EcalEndcapPRemnantClusters", "EcalEndcapPExpectedClusters",
       "EcalEndcapPTrackExpectedClusterMatches"},
      {.energyFractionToSubtract = 1.0, .defaultPDG = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "LFHCALRemnantClusters",
      {"LFHCALTrackSplitMergeClusterMatches", "LFHCALClusters", "CalorimeterTrackProjections"},
      {"LFHCALRemnantClusters", "LFHCALExpectedClusters", "LFHCALTrackExpectedClusterMatches"},
      {.energyFractionToSubtract = 1.0, .defaultPDG = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "HcalEndcapPInsertRemnantClusters",
      {"HcalEndcapPInsertTrackSplitMergeClusterMatches", "HcalEndcapPInsertClusters",
       "CalorimeterTrackProjections"},
      {"HcalEndcapPInsertRemnantClusters", "HcalEndcapPInsertExpectedClusters",
       "HcalEndcapPInsertTrackExpectedClusterMatches"},
      {.energyFractionToSubtract = 1.0, .defaultPDG = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));
}
} // extern "C"
