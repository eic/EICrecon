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
      {"EventHeader"},                 //  Inputs now include EventHeader for seeding RNG
      {"SiEndcapTrackerNoiseRawHits"}, // Output: noise-only collection
      {.addNoise               = false,
       .layer_id               = {1, 1, 2, 3, 4, 1, 1, 2, 3, 4},
       .n_noise_hits_per_layer = {405, 1442, 1442, 1440, 1435, 405, 1442, 1441, 1429, 1414},
       .detector_names = {"InnerTrackerEndcapN", "MiddleTrackerEndcapN", "OuterTrackerEndcapN",
                          "OuterTrackerEndcapN", "OuterTrackerEndcapN", "InnerTrackerEndcapP",
                          "MiddleTrackerEndcapP", "OuterTrackerEndcapP", "OuterTrackerEndcapP",
                          "OuterTrackerEndcapP"},
       .readout_name   = "TrackerEndcapHits"},
      app));
  app->Add(new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::RawTrackerHit>>(
      "SiEndcapTrackerRawHitsWithNoise", {"SiEndcapTrackerRawHits", "SiEndcapTrackerNoiseRawHits"},
      {"SiEndcapTrackerRawHitsWithNoise"}, {}, app));
  // Convert raw digitized hits into hits with geometry info (ready for tracking)
  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      "SiEndcapTrackerRecHits", {"SiEndcapTrackerRawHitsWithNoise"}, {"SiEndcapTrackerRecHits"},
      {}, // default config
      app));
}
} // extern "C"
