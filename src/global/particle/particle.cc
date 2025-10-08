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

  // backward -----------------------------------------------------------

#if EDM4EIC_VERSION_MAJOR >= 8
  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "EcalEndcapNRemnantClusters",
      {"EcalEndcapNTrackClusterMatches", "EcalEndcapNClusters", "CalorimeterTrackProjections"},
      {"EcalEndcapNRemnantClusters", "EcalEndcapNExpectedClusters",
       "EcalEndcapNTrackExpectedClusterMatches"},
      {.fracEnergyToSub = 1.0, .defaultMassPdg = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "HcalEndcapNRemnantClusters",
      {"HcalEndcapNTrackClusterMatches", "HcalEndcapNClusters", "CalorimeterTrackProjections"},
      {"HcalEndcapNRemnantClusters", "HcalEndcapNExpectedClusters",
       "HcalEndcapNTrackExpectedClusterMatches"},
      {.fracEnergyToSub = 1.0, .defaultMassPdg = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  // central ------------------------------------------------------------

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "EcalBarrelRemnantClusters",
      {"EcalBarrelTrackClusterMatches", "EcalBarrelClusters", "CalorimeterTrackProjections"},
      {"EcalBarrelRemnantClusters", "EcalBarrelExpectedClusters",
       "EcalBarrelTrackExpectedClusterMatches"},
      {.fracEnergyToSub = 1.0, .defaultMassPdg = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "HcalBarrelRemnantClusters",
      {"HcalBarrelTrackClusterMatches", "HcalBarrelClusters", "CalorimeterTrackProjections"},
      {"HcalBarrelRemnantClusters", "HcalBarrelExpectedClusters",
       "HcalBarrelTrackExpectedClusterMatches"},
      {.fracEnergyToSub = 1.0, .defaultMassPdg = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  // forward ------------------------------------------------------------

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "EcalEndcapPRemnantClusters",
      {"EcalEndcapPTrackClusterMatches", "EcalEndcapPClusters", "CalorimeterTrackProjections"},
      {"EcalEndcapPRemnantClusters", "EcalEndcapPExpectedClusters",
       "EcalEndcapPTrackExpectedClusterMatches"},
      {.fracEnergyToSub = 1.0, .defaultMassPdg = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "LFHCALRemnantClusters",
      {"LFHCALTrackSplitMergeClusterMatches", "LFHCALClusters", "CalorimeterTrackProjections"},
      {"LFHCALRemnantClusters", "LFHCALExpectedClusters", "LFHCALTrackExpectedClusterMatches"},
      {.fracEnergyToSub = 1.0, .defaultMassPdg = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "HcalEndcapPInsertRemnantClusters",
      {"HcalEndcapPInsertTrackSplitMergeClusterMatches", "HcalEndcapPInsertClusters", "CalorimeterTrackProjections"},
      {"HcalEndcapPInsertRemnantClusters", "HcalEndcapPInsertExpectedClusters", "HcalEndcapPInsertTrackExpectedClusterMatches"},
      {.fracEnergyToSub = 1.0, .defaultMassPdg = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

#endif

}
} // extern "C"
