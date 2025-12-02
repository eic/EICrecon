// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson, Subhadip Pal

#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <string>
#include <vector>

#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/meta/CollectionCollector_factory.h"
#include "factories/particle/CaloRemnantCombiner_factory.h"

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
  // PFA (2) arbitration: combine remnants, form neutral candidates
  // --------------------------------------------------------------------

  // backward -----------------------------------------------------------

  app->Add(new JOmniFactoryGeneratorT<CaloRemnantCombiner_factory>(
      "EndcapNNeutralCandidateParticlesAlpha", {"EcalEndcapNClusters", "HcalEndcapNClusters"},
      {"EndcapNNeutralCandidateParticlesAlpha"}, {.deltaRAddEM = 0.03, .deltaRAddH = 0.15}, app));

  // central ------------------------------------------------------------
  
  app->Add(new JOmniFactoryGeneratorT<CaloRemnantCombiner_factory>(
      "BarrelNeutralCandidateParticlesAlpha", {"EcalBarrelClusters", "HcalBarrelClusters"},
      {"BarrelNeutralCandidateParticlesAlpha"}, {.deltaRAddEM = 0.03, .deltaRAddH = 0.15}, app));


  // forward ------------------------------------------------------------

  app->Add(
      new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::Cluster, false>>(
          "HcalEndcapPClusters",
          {"LFHCALClusters", "HcalEndcapPInsertClusters"},
          {"HcalEndcapPClusters"}, app));

  app->Add(new JOmniFactoryGeneratorT<CaloRemnantCombiner_factory>(
      "EndcapPNeutralCandidateParticlesAlpha", {"EcalEndcapPClusters","HcalEndcapPClusters"},
      {"EndcapPNeutralCandidateParticlesAlpha"}, {.deltaRAddEM = 0.03, .deltaRAddH = 0.15}, app));
}
} // extern "C"
