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
    auto B0TrackerTimeResolution    =   0.02 * dd4hep::ns; // 15-20 ps
    auto B0TrackerIntegrationWindow =  0.750 * dd4hep::ns; // 750 ps shaping time
    app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
        "B0TrackerRawHits",
        {
          "B0TrackerHits"
        },
        {
          "B0TrackerRawHits",
          "B0TrackerHitAssociations"
        },
        {
            .threshold = 10.0 * dd4hep::keV,
            .timeResolution = B0TrackerTimeResolution,
            .integrationWindow = B0TrackerIntegrationWindow,
        },
        app
    ));

    // Convert raw digitized hits into hits with geometry info (ready for tracking)
    app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
        "B0TrackerRecHits",
        {"B0TrackerRawHits"},
        {"B0TrackerRecHits"},
        {
            .timeResolution = B0TrackerTimeResolution,
        },
        app
    ));

}
} // extern "C"
