// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <string>

#include "extensions/jana/JChainMultifactoryGeneratorT.h"
#include "factories/digi/SiliconTrackerDigi_factoryT.h"
#include "factories/tracking/TrackerHitReconstruction_factoryT.h"
#include "services/io/podio/JFactoryPodioT.h"

extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    // Digitization
    app->Add(new JChainMultifactoryGeneratorT<SiliconTrackerDigi_factoryT>(
        "B0TrackerRawHits",
        {"B0TrackerHits"},
        {"B0TrackerRawHits"},
        {
            .threshold = 1.0 * dd4hep::keV,
            .timeResolution = 8,
        },
        app
    ));

    // Convert raw digitized hits into hits with geometry info (ready for tracking)
    app->Add(new JChainMultifactoryGeneratorT<TrackerHitReconstruction_factoryT>(
        "B0TrackerRecHits",
        {"B0TrackerRawHits"},
        {"B0TrackerRecHits"},
        {
            .timeResolution = 8,
        },
        app
    ));

}
} // extern "C"
