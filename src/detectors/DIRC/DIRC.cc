// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Christopher Dilks, Nilanga Wickramaarachchi, Dmitry Kalinkin

#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <edm4hep/SimTrackerHit.h>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// algorithm configurations
#include "algorithms/digi/PhotoMultiplierHitDigiConfig.h"
#include "algorithms/meta/SubDivideFunctors.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
// factories
#include "factories/digi/PhotoMultiplierHitDigi_factory.h"
#include "factories/meta/CollectionCollector_factory.h"
#include "factories/meta/SubDivideCollection_factory.h"

extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);

  using namespace eicrecon;

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

  // MCP photon hits
  // - older geometries: DIRCBarHits readout contains only MCP photon hits
  // - newer geometries: DIRCHits readout contains MCP photon hits (surface=0)
  //   and charged particle hits in the quartz bars (surface=1), which are split
  //   into DIRCMCPHits and DIRCQuartzHits (since DIRCBarHits is already taken)
  app->Add(new JOmniFactoryGeneratorT<SubDivideCollection_factory<edm4hep::SimTrackerHit, true>>(
      "DIRCSurfaceHits", {"DIRCHits"}, {"DIRCMCPHits", "DIRCQuartzHits"},
      {
          .function = GeometrySplit{{{0}, {1}}, "DIRCHits", {"surface"}},
      },
      app));
  app->Add(new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4hep::SimTrackerHit, true>>(
      "DIRCPhotonHits", {"DIRCBarHits", "DIRCMCPHits"}, {"DIRCPhotonHits"}, app));

  // digitization
  app->Add(new JOmniFactoryGeneratorT<PhotoMultiplierHitDigi_factory>(
      "DIRCRawHits", {"EventHeader", "DIRCPhotonHits"},
      {"DIRCRawHits", "DIRCRawHitsLinks", "DIRCRawHitsAssociations"}, digi_cfg, app));
}
}
