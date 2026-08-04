// Copyright 2022, Dmitry Romanov, Minjung Kim, Joshua Sobaljic, Shujie Li
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JEventLevel.h>
#include <JANA/Utils/JTypeInfo.h>
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
  const bool split_timeframes =
      app->RegisterParameter<bool>("split_timeframes", false, "Enable timeframe splitting");
  const auto hit_level = split_timeframes ? JEventLevel::Timeslice : JEventLevel::PhysicsEvent;

  // Digitization
  app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
      "SiBarrelRawHits", {"EventHeader", "SiBarrelHits"},
      {"SiBarrelRawHits", "SiBarrelRawHitLinks", "SiBarrelRawHitAssociations"},
      {
          .threshold = 0.54 * dd4hep::keV,
      },
      app, hit_level));

  // Per-pixel noise occupancy for the barrel silicon tracker. Configurable via
  // SiBarrelNoiseRawHits:noise_rate_per_pixel_per_event (default 2e-7).
  if (!split_timeframes) {
    app->Add(new JOmniFactoryGeneratorT<RandomNoisePixel_factory>(
        "SiBarrelNoiseRawHits", {"EventHeader"}, {"SiBarrelNoiseRawHits"},
        {.addNoise = true, .noise_rate_per_pixel_per_event = 2.0e-7, .readout_name = "SiBarrelHits"},
        app));

    app->Add(new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::RawTrackerHit>>(
        "SiBarrelRawHitsWithNoise",                  // Name of the combiner instance
        {"SiBarrelRawHits", "SiBarrelNoiseRawHits"}, // Inputs: original + noise-only
        {"SiBarrelRawHitsWithNoise"},                // Output: merged collection
        {},                                          // default config
        app));
  }

  // Convert raw digitized hits into hits with geometry info (ready for tracking)
  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      "SiBarrelTrackerRecHits", {split_timeframes ? "SiBarrelRawHits" : "SiBarrelRawHitsWithNoise"},
      {"SiBarrelTrackerRecHits"}, {}, // default config
      app, hit_level));
}
} // extern "C"
