// Copyright 2022, Dmitry Romanov, Minjung Kim, Joshua Sobaljic, Shujie Li
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/RawTrackerHit.h>
#include <memory>
#include <string>
#include <vector>

#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/digi/RandomNoisePixel_factory.h"
#include "factories/digi/SiliconTrackerDigi_factory.h"
#include "factories/meta/CollectionCollector_factory.h"
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

  // Pixel occupancy is configured once for the complete SVT through
  // SVT:noise_rate_per_pixel_per_event (default 2e-7 per pixel per event).
  app->Add(new JOmniFactoryGeneratorT<RandomNoisePixel_factory>(
      "SiBarrelNoiseRawHits", {"EventHeader"}, {"SiBarrelNoiseRawHits"},
      {.addNoise = false, .readout_name = "SiBarrelHits"}, app));

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
