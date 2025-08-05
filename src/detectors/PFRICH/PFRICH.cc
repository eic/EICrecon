// Copyright (C) 2022, 2023, Christopher Dilks, Luigi Dello Stritto
// Subject to the terms in the LICENSE file found in the top-level directory.

// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Dmitry Kalinkin

#include <Evaluator/DD4hepUnits.h>
#include <JANA/Components/JOmniFactoryGeneratorT.h>
#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JEventLevel.h>
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
  digi_cfg.detectorName    = "RICHEndcapN";
  digi_cfg.readoutClass    = "RICHEndcapNHits";
  digi_cfg.hitTimeWindow   = 20.0;     // [ns]
  digi_cfg.timeResolution  = 1 / 16.0; // [ns]
  digi_cfg.speMean         = 80.0;
  digi_cfg.speError        = 16.0;
  digi_cfg.pedMean         = 200.0;
  digi_cfg.pedError        = 3.0;
  digi_cfg.enablePixelGaps = true;
  digi_cfg.safetyFactor    = 0.7;
  digi_cfg.enableNoise     = false;
  digi_cfg.noiseRate       = 20000;             // [Hz]
  digi_cfg.noiseTimeWindow = 20.0 * dd4hep::ns; // [ns]
  digi_cfg.quantumEfficiency.clear();
  digi_cfg.quantumEfficiency = {// wavelength units are [nm]
                                {315, 0.00}, {325, 0.04}, {340, 0.10}, {350, 0.20}, {370, 0.30},
                                {400, 0.35}, {450, 0.40}, {500, 0.38}, {550, 0.35}, {600, 0.27},
                                {650, 0.20}, {700, 0.15}, {750, 0.12}, {800, 0.08}, {850, 0.06},
                                {900, 0.04}, {1000, 0.00}};

  // digitization
  app->Add(new JOmniFactoryGeneratorT<PhotoMultiplierHitDigi_factory>(
      "RICHEndcapNRawHits", {"EventHeader", "RICHEndcapNHits"},
      {"RICHEndcapNRawHits", "RICHEndcapNRawHitsAssociations"}, digi_cfg));
}
}
