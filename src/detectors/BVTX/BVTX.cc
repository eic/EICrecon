// Copyright 2022, Dmitry Romanov, Minjung Kim, Joshua Sobaljic, Shujie Li

// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <edm4eic/RawTrackerHit.h>
#include <memory>
#include <string>
#include <vector>

#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/digi/RandomNoisePixel_factory.h"
#include "factories/digi/SiliconTrackerDigi_factory.h"
// Use precompiled digi factory instantiations to reduce build time
#include "factories/digi/generated/factories.h"
#include "factories/meta/CollectionCollector_factory.h"
#include "factories/tracking/TrackerHitReconstruction_factory.h"
// Use precompiled tracking factory instantiations to reduce build time
#include "factories/tracking/generated/factories.h"

extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);

  using namespace eicrecon;

  // Digitization
  app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
      "SiBarrelVertexRawHits", {"EventHeader", "VertexBarrelHits"},
      {"SiBarrelVertexRawHits", "SiBarrelVertexRawHitLinks", "SiBarrelVertexRawHitAssociations"},
      {
          .threshold = 0.54 * dd4hep::keV,
      },
      app));
  app->Add(new JOmniFactoryGeneratorT<RandomNoisePixel_factory>(
      "SiBarrelVertexNoiseRawHits", {"EventHeader"}, {"SiBarrelVertexNoiseRawHits"},
      {.addNoise                       = true,
       .noise_rate_per_pixel_per_event = 2.0e-7,
       .readout_name                   = "VertexBarrelHits"},
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
