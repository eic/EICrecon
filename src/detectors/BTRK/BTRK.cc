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
#include "factories/meta/CollectionCollector_factory.h"

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
      "SiBarrelNoiseRawHits", // 1. Instance name (noise-only producer)
      {"EventHeader"}, // 2. No input collection but Event header for random generator (source-mode)
      {"SiBarrelNoiseRawHits"}, // 3. Output: noise-only collection
      {.addNoise = false, .n_noise_hits_per_layer = {1145, 2639}, .readout_name = "SiBarrelHits", .layer_id = {1, 1}, .detector_names = {"SagittaSiBarrel", "OuterSiBarrel"}}, app));

  app->Add(new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::RawTrackerHit>>(
      "SiBarrelRawHitsWithNoise",                  // Name of the combiner instance
      {"SiBarrelRawHits", "SiBarrelNoiseRawHits"}, // Inputs: original + noise-only
      {"SiBarrelRawHitsWithNoise"},                // Output: merged collection
      {},                                          // default config
      app));

  // Convert raw digitized hits into hits with geometry info (ready for tracking)
  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      "SiBarrelTrackerRecHits", {"SiBarrelRawHitsWithNoise"}, {"SiBarrelTrackerRecHits"},
      {}, // default config
      app));
}
} // extern "C"
