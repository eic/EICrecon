// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/TrackClusterMatch.h>
#include <edm4eic/TrackPoint.h>
#include <edm4eic/TrackSegment.h>
#include <podio/RelationRange.h>
#include <cstdint>
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/meta/CollectionCollector_factory.h"
#include "factories/meta/SubDivideCollection_factory.h"
#include "factories/particle_flow/ChargedCandidateMaker_factory.h"
#include "factories/particle_flow/TrackClusterSubtractor_factory.h"
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
      {"HcalEndcapNSplitMergeProtoClusters", "HcalEndcapNSplitMergeClusters"},
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
  // PFA (1a) arbitration: apply track correction to clusters
  // --------------------------------------------------------------------

  std::vector<uint32_t> systemIDs{
      103, //< EEEMCal ID
      113, //< NHCal ID
      101, //< BIC ID
      111, //< BHCal ID
      102, //< FEMC ID
      116, //< LFHCAL ID
      115  //< FHCal Insert ID
  };

  auto subDivideBySystemID =
      [systemIDs](const edm4eic::TrackSegment& projection) -> std::vector<int> {
    std::vector<int> indices;
    const auto& points = projection.getPoints();
    for (std::size_t iSysID = 0; iSysID < systemIDs.size(); ++iSysID) {
      for (const auto& point : points) {
        if (point.system == systemIDs[iSysID]) {
          indices.push_back(iSysID);
          break;
        }
      }
    }
    return indices;
  };

  app->Add(new JOmniFactoryGeneratorT<SubDivideCollection_factory<edm4eic::TrackSegment>>(
      "EndcapPBarrelEndcapNCalorimeterTrackProjections", {"CalorimeterTrackProjections"},
      {"EcalEndcapNCalorimeterTrackProjections", "HcalEndcapNCalorimeterTrackProjections",
       "EcalBarrelCalorimeterTrackProjections", "HcalBarrelCalorimeterTrackProjections",
       "EcalEndcapPCalorimeterTrackProjections", "LFHCALTrackProjections",
       "HcalEndcapPInsertCalorimeterTrackProjections"},
      {.function = subDivideBySystemID}, app));

  // backward -----------------------------------------------------------

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "EcalEndcapNRemnantClusters",
      {"EcalEndcapNTrackClusterMatches", "EcalEndcapNClusters",
       "EcalEndcapNCalorimeterTrackProjections"},
      {"EcalEndcapNRemnantClusters", "EcalEndcapNExpectedClusters",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "EcalEndcapNTrackExpectedClusterLinks",
#endif
       "EcalEndcapNTrackExpectedClusterMatches"},
      {.energyFractionToSubtract = 1.0, .defaultPDG = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "HcalEndcapNRemnantClusters",
      {"HcalEndcapNTrackClusterMatches", "HcalEndcapNClusters",
       "HcalEndcapNCalorimeterTrackProjections"},
      {"HcalEndcapNRemnantClusters", "HcalEndcapNExpectedClusters",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "HcalEndcapNTrackExpectedClusterLinks",
#endif
       "HcalEndcapNTrackExpectedClusterMatches"},
      {.energyFractionToSubtract = 1.0, .defaultPDG = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  // central ------------------------------------------------------------

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "EcalBarrelRemnantClusters",
      {"EcalBarrelTrackClusterMatches", "EcalBarrelClusters",
       "EcalBarrelCalorimeterTrackProjections"},
      {"EcalBarrelRemnantClusters", "EcalBarrelExpectedClusters",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "EcalBarrelTrackExpectedClusterLinks",
#endif
       "EcalBarrelTrackExpectedClusterMatches"},
      {.energyFractionToSubtract = 1.0, .defaultPDG = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "HcalBarrelRemnantClusters",
      {"HcalBarrelTrackClusterMatches", "HcalBarrelClusters",
       "HcalBarrelCalorimeterTrackProjections"},
      {"HcalBarrelRemnantClusters", "HcalBarrelExpectedClusters",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "HcalBarrelTrackExpectedClusterLinks",
#endif
       "HcalBarrelTrackExpectedClusterMatches"},
      {.energyFractionToSubtract = 1.0, .defaultPDG = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  // forward ------------------------------------------------------------

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "EcalEndcapPRemnantClusters",
      {"EcalEndcapPTrackClusterMatches", "EcalEndcapPClusters",
       "EcalEndcapPCalorimeterTrackProjections"},
      {"EcalEndcapPRemnantClusters", "EcalEndcapPExpectedClusters",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "EcalEndcapPTrackExpectedClusterLinks",
#endif
       "EcalEndcapPTrackExpectedClusterMatches"},
      {.energyFractionToSubtract = 1.0, .defaultPDG = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "LFHCALRemnantClusters",
      {"LFHCALTrackSplitMergeClusterMatches", "LFHCALClusters", "LFHCALTrackProjections"},
      {"LFHCALRemnantClusters", "LFHCALExpectedClusters",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "LFHCALTrackExpectedClusterLinks",
#endif
       "LFHCALTrackExpectedClusterMatches"},
      {.energyFractionToSubtract = 1.0, .defaultPDG = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "HcalEndcapPInsertRemnantClusters",
      {"HcalEndcapPInsertTrackSplitMergeClusterMatches", "HcalEndcapPInsertClusters",
       "HcalEndcapPInsertCalorimeterTrackProjections"},
      {"HcalEndcapPInsertRemnantClusters", "HcalEndcapPInsertExpectedClusters",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "HcalEndcapPInsertTrackExpectedClusterLinks",
#endif
       "HcalEndcapPInsertTrackExpectedClusterMatches"},
      {.energyFractionToSubtract = 1.0, .defaultPDG = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

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
