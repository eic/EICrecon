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
    auto TOFEndcapTimeResolution    =  20.0 * dd4hep::ns; // 20 ps bin width
    auto TOFEndcapIntegrationWindow =   5.0 * dd4hep::ns; // shaping time
    app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
      "TOFEndcapRawHits",
      {
        "TOFEndcapHits"},
      {
        "TOFEndcapRawHits",
        "TOFEndcapHitAssociations"
      },
      {
        .threshold = 6.0 * dd4hep::keV,
        .timeResolution = TOFEndcapTimeResolution,
        .integrationWindow = TOFEndcapIntegrationWindow,

      },
      app
    ));

    // Convert raw digitized hits into hits with geometry info (ready for tracking)
    app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      "TOFEndcapRecHits",
      {"TOFEndcapRawHits"},     // Input data collection tags
      {"TOFEndcapRecHits"},     // Output data tag
      {
        .timeResolution = TOFEndcapTimeResolution,
      },
      app
    ));

}
} // extern "C"
