// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 EICrecon authors

#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>

#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/arbor/ArborPFA_factory.h"

extern "C" {

void InitPlugin(JApplication* app) {

  using namespace eicrecon;

  InitJANAPlugin(app);

  // ====================================================================
  // ArborPFA: Tree-based Pandora-based particle flow analysis
  // ====================================================================
  // ArborPFA uses tree-building algorithms optimized for highly granular
  // calorimeters (e.g., dual-readout, fine-grained ECAL/HCAL).
  // This runs in parallel to PandoraPFA for comparison.

  app->Add(new JOmniFactoryGeneratorT<ArborPFA_factory>(
      "ArborPFAParticles",
      {"EcalBarrelRecHits", "EcalEndcapRecHits", "HcalBarrelRecHits", "HcalEndcapRecHits",
       "CalorimeterTrackProjections"},
      {"ArborPFAParticles"}, {.pandoraSettingsFile = "ArborPFASettings.xml"}, app));
}
} // extern "C"
