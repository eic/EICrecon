// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <edm4eic/EDM4eicVersion.h>
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
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
      {"EcalEndcapNTrackSplitMergeProtoClusterLinks", "EcalEndcapNSplitMergeProtoClusters",
       "EcalEndcapNSplitMergeClusters"},
#elif EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 4, 0)
      {"EcalEndcapNTrackSplitMergeProtoClusterMatches", "EcalEndcapNSplitMergeProtoClusters",
       "EcalEndcapNSplitMergeClusters"},
#else
      {"EcalEndcapNSplitMergeProtoClusters", "EcalEndcapNSplitMergeClusters"},
#endif
      {"EcalEndcapNTrackSplitMergeClusterMatches"}, {}, app));

  app->Add(new JOmniFactoryGeneratorT<TrackProtoClusterMatchPromoter_factory>(
      "HcalEndcapNTrackSplitMergeClusterMatches",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
      {"HcalEndcapNTrackSplitMergeProtoClusterLinks", "HcalEndcapNSplitMergeProtoClusters",
       "HcalEndcapNSplitMergeClusters"},
#elif EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 4, 0)
      {"HcalEndcapNTrackSplitMergeProtoClusterMatches", "HcalEndcapNSplitMergeProtoClusters",
       "HcalEndcapNSplitMergeClusters"},
#else
      {"HcalEndcapNTrackSplitMergeProtoClusterMatches", "HcalEndcapNSplitMergeProtoClusters"},
#endif
      {"HcalEndcapNTrackSplitMergeClusterMatches"}, {}, app));

  // central ------------------------------------------------------------

  app->Add(new JOmniFactoryGeneratorT<TrackProtoClusterMatchPromoter_factory>(
      "HcalBarrelTrackSplitMergeClusterMatches",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
      {"HcalBarrelTrackSplitMergeProtoClusterLinks", "HcalBarrelSplitMergeProtoClusters",
       "HcalBarrelSplitMergeClusters"},
#elif EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 4, 0)
      {"HcalBarrelTrackSplitMergeProtoClusterMatches", "HcalBarrelSplitMergeProtoClusters",
       "HcalBarrelSplitMergeClusters"},
#else
      {"HcalBarrelSplitMergeProtoClusters", "HcalBarrelSplitMergeClusters"},
#endif
      {"HcalBarrelTrackSplitMergeClusterMatches"}, {}, app));

  // forward ------------------------------------------------------------

  app->Add(new JOmniFactoryGeneratorT<TrackProtoClusterMatchPromoter_factory>(
      "EcalEndcapPTrackSplitMergeClusterMatches",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
      {"EcalEndcapPTrackSplitMergeProtoClusterLinks", "EcalEndcapPSplitMergeProtoClusters",
       "EcalEndcapPSplitMergeClusters"},
#elif EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 4, 0)
      {"EcalEndcapPTrackSplitMergeProtoClusterMatches", "EcalEndcapPSplitMergeProtoClusters",
       "EcalEndcapPSplitMergeClusters"},
#else
      {"EcalEndcapPSplitMergeProtoClusters", "EcalEndcapPSplitMergeClusters"},
#endif
      {"EcalEndcapPTrackSplitMergeClusterMatches"}, {}, app));

  app->Add(new JOmniFactoryGeneratorT<TrackProtoClusterMatchPromoter_factory>(
      "LFHCALTrackSplitMergeClusterMatches",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
      {"LFHCALTrackSplitMergeProtoClusterLinks", "LFHCALSplitMergeProtoClusters",
       "LFHCALSplitMergeClusters"},
#elif EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 4, 0)
      {"LFHCALTrackSplitMergeProtoClusterMatches", "LFHCALSplitMergeProtoClusters",
       "LFHCALSplitMergeClusters"},
#else
      {"LFHCALSplitMergeProtoClusters", "LFHCALSplitMergeClusters"},
#endif
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
