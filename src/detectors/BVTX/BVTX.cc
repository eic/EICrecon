// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplicationFwd.h>
#include <edm4eic/EDM4eicVersion.h>
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
      "SiBarrelVertexRawHits", {"EventHeader", "VertexBarrelHits"},
      {"SiBarrelVertexRawHits",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "SiBarrelVertexRawHitLinks",
#endif
       "SiBarrelVertexRawHitAssociations"},
      {
          .threshold = 0.54 * dd4hep::keV,
      },
      app));
  app->Add(new JOmniFactoryGeneratorT<RandomNoise_factory>(
      "SiBarrelVertexNoiseRawHits",   // Instance name (noise-only producer)
      {"EventHeader"},                // Inputs now include EventHeader for seeding RNG
      {"SiBarrelVertexNoiseRawHits"}, // Output: noise-only collection
      {.addNoise               = false,
       .n_noise_hits_per_layer = {76, 102, 254},
       .readout_name           = "VertexBarrelHits",
       .layer_id               = {1, 2, 4}},
      app));
  app->Add(new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::RawTrackerHit>>(
      "SiBarrelVertexRawHitsWithNoise", {"SiBarrelVertexRawHits", "SiBarrelVertexNoiseRawHits"},
      {"SiBarrelVertexRawHitsWithNoise"}, {}, app));

  // Convert raw digitized hits into hits with geometry info (ready for tracking)
  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      "SiBarrelVertexRecHits", {"SiBarrelVertexRawHitsWithNoise"}, {"SiBarrelVertexRecHits"},
      {}, // default config
      app));
}
} // extern "C"
