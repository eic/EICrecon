// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <edm4eic/EDM4eicVersion.h>
#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplicationFwd.h>
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

  // Digitization
  app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
      "SiBarrelRawHits", {"EventHeader", "SiBarrelHits"},
      {"SiBarrelRawHits",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "SiBarrelRawHitLinks",
#endif
       "SiBarrelRawHitAssociations"},
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
