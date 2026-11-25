// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Christopher Dilks, Nilanga Wickramaarachchi, Dmitry Kalinkin

#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <edm4hep/SimTrackerHit.h>
#include <functional>
#include <memory>
#include <string>
#include <vector>

// algorithm configurations
#include "algorithms/digi/DircGRdigConfig.h"
#include "algorithms/meta/SubDivideFunctors.h"
#include "algorithms/pid/DircGRConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
// factories
#include "factories/digi/DircGRdig_factory.h"
#include "factories/meta/CollectionCollector_factory.h"
#include "factories/meta/SubDivideCollection_factory.h"
#include "factories/pid/DircGR_factory.h"
#include "factories/pid/DircTruthIncidence_factory.h"

extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);

  using namespace eicrecon;

  // configuration parameters ///////////////////////////////////////////////

  // digitization (quantum efficiency is applied in npsim, stacking action).
  // The TDC LSB is shared: DircGR converts the raw-hit timeStamps back to ns.
  DircGRdigConfig digi_cfg;
  DircGRConfig gr_cfg;
  gr_cfg.timeResolution = digi_cfg.timeResolution;

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
  app->Add(new JOmniFactoryGeneratorT<DircGRdig_factory>(
      "DIRCRawHits", {"EventHeader", "DIRCPhotonHits"},
      {"DIRCRawHits", "DIRCRawHitsLinks", "DIRCRawHitsAssociations"}, digi_cfg, app));

  // hpDIRC GR: truth bar incidence (first bar entry per charged particle)
  // from the quartz-bar hits; input to DircGR at the truth stage
  app->Add(new JOmniFactoryGeneratorT<DircTruthIncidence_factory>(
      "DircTruthIncidence", {"DIRCQuartzHits"}, {"DircTruthTrackSegments", "DircIncidenceHits"}, {},
      app));

  // hpDIRC GR (geometric reconstruction): bar-entry segments from the
  // quartz-bar hits + digitized MCP hits (pixel = cellID; links only for the
  // calibration dump). LUTs: -PDIRC:DircGR:lutPath, -PDIRC:DircGR:chrcorrFile.
  app->Add(new JOmniFactoryGeneratorT<DircGR_factory>(
      "DircGR", {"DircTruthTrackSegments", "DIRCRawHits", "DIRCRawHitsLinks"},
      {"DircGRParticleIDs", "DircGRRadiatorInfo"}, gr_cfg, app));
}
}
