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
      "SiEndcapTrackerRawHits", {"EventHeader", "TrackerEndcapHits"},
      {"SiEndcapTrackerRawHits", "SiEndcapTrackerRawHitAssociations"},
      {
          .threshold = 0.54 * dd4hep::keV,
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<RandomNoise_factory>(
      "SiEndcapTrackerNoiseRawHits",   // Instance name (noise-only producer)
      {"EventHeader"},                                //  Inputs now include EventHeader for seeding RNG
      {"SiEndcapTrackerNoiseRawHits"}, // Output: noise-only collection
      {.addNoise                = true,
       .n_noise_hits_per_system = 11820,
       .readout_name            = "TrackerEndcapHits"},
      app));
  app->Add(new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::RawTrackerHit>>(
      "SiEndcapTrackerRawHitsWithNoise",
      {"SiEndcapTrackerRawHits", "SiEndcapTrackerNoiseRawHits"},
      {"SiEndcapTrackerRawHitsWithNoise"},
      {},
      app));
  // Convert raw digitized hits into hits with geometry info (ready for tracking)
  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      "SiEndcapTrackerRecHits", {"SiEndcapTrackerRawHitsWithNoise"}, {"SiEndcapTrackerRecHits"},
      {}, // default config
      app));
}
} // extern "C"
