// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <string>

#include "algorithms/interfaces/WithPodConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/digi/SiliconTrackerDigi_factory.h"
#include "factories/tracking/TrackerHitReconstruction_factory.h"

extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    // Digitization
    app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
      "TOFEndcapRawHits",
      {
        "TOFEndcapHits"},
      {
        "TOFEndcapRawHits",
        "TOFEndcapRawHitAssociations"
      },
      {
        .threshold = 6.0 * dd4hep::keV,
        .timeResolution = 0.025,
      },
      app
    ));

    // Convert raw digitized hits into hits with geometry info (ready for tracking)
    app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      "TOFEndcapRecHits",
      {"TOFEndcapRawHits"},     // Input data collection tags
      {"TOFEndcapRecHits"},     // Output data tag
      {
        .timeResolution = 0.025,
      },
      app
    ));

}
} // extern "C"
