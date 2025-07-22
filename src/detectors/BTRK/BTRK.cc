// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <string>
#include <vector>

#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/digi/SiliconTrackerDigi_factory.h"
#include "factories/digi/RandomNoise_factory.h"
#include "factories/tracking/TrackerHitReconstruction_factory.h"

extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);

  using namespace eicrecon;

  // Digitization
  app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
      "SiBarrelRawHits", {"EventHeader", "SiBarrelHits"},
      {"SiBarrelRawHits", "SiBarrelRawHitAssociations"},
      {
          .threshold = 0.54 * dd4hep::keV,
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<RandomNoise_factory>(
      "NoisySiBarrelRawHits",   // 1. The name of the plugin instance
      {"SiBarrelRawHits"},      // 2. The input collection tag
      {"NoisySiBarrelRawHits"}, // 3. The output collection tag
      {.addNoise                = false,
       .n_noise_hits_per_system = 3784,
       .readout_name            = "SiBarrelHits"}, // 4. Use default config from your .yaml file
      app));

  // Convert raw digitized hits into hits with geometry info (ready for tracking)
  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      "SiBarrelTrackerRecHits", {"NoisySiBarrelRawHits"}, {"SiBarrelTrackerRecHits"},
      {}, // default config
      app));
}
} // extern "C"
