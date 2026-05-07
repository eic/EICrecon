// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <edm4eic/TrackClusterMatch.h>
#include <memory>
#include <string>
#include <vector>

#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/meta/CollectionCollector_factory.h"
#include "factories/particle_flow/ChargedCandidateMaker_factory.h"
#include "factories/particle_flow/TrackProtoClusterMatchPromoter_factory.h"

extern "C" {

void InitPlugin(JApplication* app) {

  using namespace eicrecon;

  InitJANAPlugin(app);

  // ====================================================================
  // PFAlpha: baseline PF implementation
  // ====================================================================

  // --------------------------------------------------------------------
  // PFA (0b) connection: promote track-protocluster links
  // --------------------------------------------------------------------

  // backward -----------------------------------------------------------

  app->Add(new JOmniFactoryGeneratorT<TrackProtoClusterMatchPromoter_factory>(
      "EcalEndcapNTrackSplitMergeClusterMatches",
      {"EcalEndcapNTrackSplitMergeProtoClusterMatches", "EcalEndcapNSplitMergeProtoClusters",
       "EcalEndcapNSplitMergeClusters"},
      {"EcalEndcapNTrackSplitMergeClusterMatches"}, {}, app));

  app->Add(new JOmniFactoryGeneratorT<TrackProtoClusterMatchPromoter_factory>(
      "HcalEndcapNTrackSplitMergeClusterMatches",
      {"HcalEndcapNTrackSplitMergeProtoClusterMatches", "HcalEndcapNSplitMergeProtoClusters",
       "HcalEndcapNSplitMergeClusters"},
      {"HcalEndcapNTrackSplitMergeClusterMatches"}, {}, app));

  // central ------------------------------------------------------------

  app->Add(new JOmniFactoryGeneratorT<TrackProtoClusterMatchPromoter_factory>(
      "HcalBarrelTrackSplitMergeClusterMatches",
      {"HcalBarrelTrackSplitMergeProtoClusterMatches", "HcalBarrelSplitMergeProtoClusters",
       "HcalBarrelSplitMergeClusters"},
      {"HcalBarrelTrackSplitMergeClusterMatches"}, {}, app));

  // forward ------------------------------------------------------------

  app->Add(new JOmniFactoryGeneratorT<TrackProtoClusterMatchPromoter_factory>(
      "EcalEndcapPTrackSplitMergeClusterMatches",
      {"EcalEndcapPTrackSplitMergeProtoClusterMatches", "EcalEndcapPSplitMergeProtoClusters",
       "EcalEndcapPSplitMergeClusters"},
      {"EcalEndcapPTrackSplitMergeClusterMatches"}, {}, app));

  app->Add(new JOmniFactoryGeneratorT<TrackProtoClusterMatchPromoter_factory>(
      "LFHCALTrackSplitMergeClusterMatches",
      {"LFHCALTrackSplitMergeProtoClusterMatches", "LFHCALSplitMergeProtoClusters",
       "LFHCALSplitMergeClusters"},
      {"LFHCALTrackSplitMergeClusterMatches"}, {}, app));

  // --------------------------------------------------------------------
  // PFA (1b) arbitration: form charged candidates
  // --------------------------------------------------------------------

  // backward -----------------------------------------------------------

  app->Add(
      new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::TrackClusterMatch, false>>(
          "EndcapNTrackClusterMatches",
          {"EcalEndcapNTrackClusterMatches", "HcalEndcapNTrackClusterMatches"},
          {"EndcapNTrackClusterMatches"}, app));

  app->Add(new JOmniFactoryGeneratorT<ChargedCandidateMaker_factory>(
      "EndcapNChargedCandidateParticlesAlpha", {"EndcapNTrackClusterMatches"},
      {"EndcapNChargedCandidateParticlesAlpha"}, {}, app));

  // central ------------------------------------------------------------

  app->Add(
      new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::TrackClusterMatch, false>>(
          "BarrelTrackClusterMatches",
          {"EcalBarrelTrackClusterMatches", "HcalBarrelTrackClusterMatches"},
          {"BarrelTrackClusterMatches"}, app));

  app->Add(new JOmniFactoryGeneratorT<ChargedCandidateMaker_factory>(
      "BarrelChargedCandidateParticlesAlpha", {"BarrelTrackClusterMatches"},
      {"BarrelChargedCandidateParticlesAlpha"}, {}, app));

  // forward ------------------------------------------------------------

  app->Add(
      new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::TrackClusterMatch, false>>(
          "EndcapPTrackClusterMatches",
          {"EcalEndcapPTrackClusterMatches", "LFHCALTrackClusterMatches",
           "HcalEndcapPInsertTrackClusterMatches"},
          {"EndcapPTrackClusterMatches"}, app));

  app->Add(new JOmniFactoryGeneratorT<ChargedCandidateMaker_factory>(
      "EndcapPChargedCandidateParticlesAlpha", {"EndcapPTrackClusterMatches"},
      {"EndcapPChargedCandidateParticlesAlpha"}, {}, app));
}
} // extern "C"
