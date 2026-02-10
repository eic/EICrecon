// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JEventLevel.h>
#include <JANA/Utils/JTypeInfo.h>
#include <string>
#include <vector>

#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/digi/SiliconTrackerDigi_factory.h"
#include "factories/tracking/TrackerHitReconstruction_factory.h"

extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);

  using namespace eicrecon;
  using eicrecon::JOmniFactoryGeneratorT;

  // Digitization
  app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
      "SiBarrelRawHits", {"EventHeader", "SiBarrelHits"},
      {"SiBarrelRawHits", "SiBarrelRawHitAssociations"},
      {
          .threshold = 0.54 * dd4hep::keV,
      },
      app));

  // Convert raw digitized hits into hits with geometry info (ready for tracking)
  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      "SiBarrelTrackerRecHits", {"SiBarrelRawHits"}, {"SiBarrelTrackerRecHits"},
      {}, // default config
      app));
}
} // extern "C"
