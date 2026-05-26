// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson, Subhadip Pal

#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <edm4eic/Cluster.h>
#include <edm4eic/TrackClusterMatch.h>
#include <memory>
#include <string>
#include <vector>

#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/meta/CollectionCollector_factory.h"
#include "factories/particle_flow/CaloRemnantCombiner_factory.h"
#include "factories/particle_flow/ChargedCandidateMaker_factory.h"

extern "C" {

void InitPlugin(JApplication* app) {

  using namespace eicrecon;

  InitJANAPlugin(app);

  // ====================================================================
  // PFAlpha: baseline PF implementation
  // ====================================================================

  // --------------------------------------------------------------------
  // PFA (2) arbitration: combine remnants, form neutral candidates
  // PFA (1b) arbitration: form charged candidates
  // --------------------------------------------------------------------

  // backward -----------------------------------------------------------

  app->Add(new JOmniFactoryGeneratorT<CaloRemnantCombiner_factory>(
      "EndcapNNeutralCandidateParticlesAlpha", {"EcalEndcapNClusters", "HcalEndcapNClusters"},
      {"EndcapNNeutralCandidateParticlesAlpha"}, {.ecalDeltaR = 0.03, .hcalDeltaR = 0.15}, app));

  // central ------------------------------------------------------------

  app->Add(new JOmniFactoryGeneratorT<CaloRemnantCombiner_factory>(
      "BarrelNeutralCandidateParticlesAlpha", {"EcalBarrelClusters", "HcalBarrelClusters"},
      {"BarrelNeutralCandidateParticlesAlpha"}, {.ecalDeltaR = 0.03, .hcalDeltaR = 0.15}, app));

  // forward ------------------------------------------------------------

  app->Add(new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::Cluster, false>>(
      "HcalEndcapPClusters", {"LFHCALClusters", "HcalEndcapPInsertClusters"},
      {"HcalEndcapPClusters"}, app));

  app->Add(new JOmniFactoryGeneratorT<CaloRemnantCombiner_factory>(
      "EndcapPNeutralCandidateParticlesAlpha", {"EcalEndcapPClusters", "HcalEndcapPClusters"},
      {"EndcapPNeutralCandidateParticlesAlpha"}, {.ecalDeltaR = 0.03, .hcalDeltaR = 0.15}, app));
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
