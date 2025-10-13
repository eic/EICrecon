// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson

#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <edm4eic/TrackClusterMatch.h>
#include <fmt/core.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/meta/CollectionCollector_factory.h"
#include "factories/particle/ChargedCandidateMaker_factory.h"
#include "factories/particle/TrackClusterSubtractor_factory.h"

extern "C" {

void InitPlugin(JApplication* app) {

  using namespace eicrecon;

  InitJANAPlugin(app);

  // ====================================================================
  // PFAlpha: baseline PF implementation
  // ====================================================================

  // --------------------------------------------------------------------
  // PFA (1a) arbitration: apply track correction to clusters
  // --------------------------------------------------------------------

  // backward -----------------------------------------------------------

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
      {"HcalEndcapPInsertTrackSplitMergeClusterMatches", "HcalEndcapPInsertClusters",
       "CalorimeterTrackProjections"},
      {"HcalEndcapPInsertRemnantClusters", "HcalEndcapPInsertExpectedClusters",
       "HcalEndcapPInsertTrackExpectedClusterMatches"},
      {.fracEnergyToSub = 1.0, .defaultMassPdg = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  // --------------------------------------------------------------------
  // PFA (1b) arbitration: form charged candidates
  // --------------------------------------------------------------------

  // backward -----------------------------------------------------------

  app->Add(
      new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::TrackClusterMatch, false>>(
          "EndcapNTrackExpectedClusterMatches",
          {"EcalEndcapNTrackExpectedClusterMatches", "HcalEndcapNTrackExpectedClusterMatches"},
          {"EndcapNTrackExpectedClusterMatches"}, app));

  app->Add(new JOmniFactoryGeneratorT<ChargedCandidateMaker_factory>(
      "EndcapNChargedCandidateParticlesAlpha", {"EndcapNTrackExpectedClusterMatches"},
      {"EndcapNChargedCandidateParticlesAlpha"}, {}, app));

  // central ------------------------------------------------------------

  app->Add(
      new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::TrackClusterMatch, false>>(
          "BarrelTrackExpectedClusterMatches",
          {"EcalBarrelTrackExpectedClusterMatches", "HcalBarrelTrackExpectedClusterMatches"},
          {"BarrelTrackExpectedClusterMatches"}, app));

  app->Add(new JOmniFactoryGeneratorT<ChargedCandidateMaker_factory>(
      "BarrelChargedCandidateParticlesAlpha", {"BarrelTrackExpectedClusterMatches"},
      {"BarrelChargedCandidateParticlesAlpha"}, {}, app));

  // forward ------------------------------------------------------------

  app->Add(
      new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::TrackClusterMatch, false>>(
          "EndcapPTrackExpectedClusterMatches",
          {"EcalEndcapPTrackExpectedClusterMatches", "LFHCALTrackExpectedClusterMatches"},
          {"EndcapPTrackExpectedClusterMatches"}, app));

  app->Add(new JOmniFactoryGeneratorT<ChargedCandidateMaker_factory>(
      "EndcapPChargedCandidateParticlesAlpha", {"EndcapPTrackExpectedClusterMatches"},
      {"EndcapPChargedCandidateParticlesAlpha"}, {}, app));

  app->Add(
      new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::TrackClusterMatch, false>>(
          "EndcapPInsertTrackExpectedClusterMatches",
          {"EcalEndcapPTrackExpectedClusterMatches",
           "HcalEndcapPInsertTrackExpectedClusterMatches"},
          {"EndcapPInsertTrackExpectedClusterMatches"}, app));

  app->Add(new JOmniFactoryGeneratorT<ChargedCandidateMaker_factory>(
      "EndcapPInsertChargedCandidateParticlesAlpha", {"EndcapPInsertTrackExpectedClusterMatches"},
      {"EndcapPInsertChargedCandidateParticlesAlpha"}, {}, app));
}
} // extern "C"
