// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Christopher Dilks, Nilanga Wickramaarachchi, Dmitry Kalinkin

#include <JANA/Components/JOmniFactoryGeneratorT.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <string>
#include <utility>
#include <vector>

// algorithm configurations
#include "algorithms/digi/PhotoMultiplierHitDigiConfig.h"
// factories
#include "factories/digi/PhotoMultiplierHitDigi_factory.h"

extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);

  using namespace eicrecon;
  using jana::components::JOmniFactoryGeneratorT;

  // configuration parameters ///////////////////////////////////////////////

  // digitization
  PhotoMultiplierHitDigiConfig digi_cfg;
  digi_cfg.hitTimeWindow   = 20.0;     // [ns]
  digi_cfg.timeResolution  = 1 / 16.0; // [ns]
  digi_cfg.speMean         = 80.0;
  digi_cfg.speError        = 16.0;
  digi_cfg.pedMean         = 200.0;
  digi_cfg.pedError        = 3.0;
  digi_cfg.enablePixelGaps = false;
  digi_cfg.safetyFactor    = 1.0;
  // Actual efficiency is implemented in npsim via a stacking action
  digi_cfg.enableQuantumEfficiency = false;
  digi_cfg.quantumEfficiency.clear();

  // digitization
  app->Add(new JOmniFactoryGeneratorT<PhotoMultiplierHitDigi_factory>(
      "DIRCRawHits", {"EventHeader", "DIRCBarHits"}, {"DIRCRawHits", "DIRCRawHitsAssociations"},
      digi_cfg));
}
}
