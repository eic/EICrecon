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

extern "C" {

void InitPlugin(JApplication* app) {

  using namespace eicrecon;

  InitJANAPlugin(app);

  // ====================================================================
  // PFAlpha: baseline PF implementation
  // ====================================================================

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
          {"EcalEndcapPTrackClusterMatches", "LFHCALTrackClusterMatches"},
          {"EndcapPTrackClusterMatches"}, app));

  app->Add(new JOmniFactoryGeneratorT<ChargedCandidateMaker_factory>(
      "EndcapPChargedCandidateParticlesAlpha", {"EndcapPTrackClusterMatches"},
      {"EndcapPChargedCandidateParticlesAlpha"}, {}, app));

  app->Add(
      new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::TrackClusterMatch, false>>(
          "EndcapPInsertTrackClusterMatches",
          {"EcalEndcapPTrackClusterMatches", "HcalEndcapPInsertTrackClusterMatches"},
          {"EndcapPInsertTrackClusterMatches"}, app));

  app->Add(new JOmniFactoryGeneratorT<ChargedCandidateMaker_factory>(
      "EndcapPInsertChargedCandidateParticlesAlpha", {"EndcapPInsertTrackClusterMatches"},
      {"EndcapPInsertChargedCandidateParticlesAlpha"}, {}, app));
}
} // extern "C"
