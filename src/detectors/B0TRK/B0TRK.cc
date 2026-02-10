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
      "B0TrackerRawHits", {"EventHeader", "B0TrackerHits"},
      {"B0TrackerRawHits", "B0TrackerRawHitAssociations"},
      {
          .threshold      = 10.0 * dd4hep::keV,
          .timeResolution = 8,
      },
      app));

  // Convert raw digitized hits into hits with geometry info (ready for tracking)
  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      "B0TrackerRecHits", {"B0TrackerRawHits"}, {"B0TrackerRecHits"},
      {
          .timeResolution = 8,
      },
      app));
}
} // extern "C"
