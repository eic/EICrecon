// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 EICrecon authors

#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>

#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/pandora/PandoraPFA_factory.h"

extern "C" {

void InitPlugin(JApplication* app) {

  using namespace eicrecon;

  InitJANAPlugin(app);

  // ====================================================================
  // PandoraPFA: Pandora-based particle flow analysis
  // ====================================================================
  // Note: Using actual EICrecon collection names (barrel has Imaging+ScFi, endcaps split N/P)

  app->Add(new JOmniFactoryGeneratorT<PandoraPFA_factory>(
      "PandoraPFAParticles",
      {"EcalBarrelImagingRecHits", "EcalBarrelScFiRecHits", "EcalEndcapNRecHits",
       "EcalEndcapPRecHits", "HcalBarrelRecHits", "HcalEndcapNRecHits",
       "CalorimeterTrackProjections"},
      {"PandoraPFAParticles"}, {.pandoraSettingsFile = "PandoraPFASettings.xml"}, app));
}
} // extern "C"
