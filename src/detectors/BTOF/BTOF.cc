// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>

#include "extensions/jana/JChainMultifactoryGeneratorT.h"

#include "factories/digi/SiliconTrackerDigi_factoryT.h"
#include "factories/tracking/TrackerHitReconstruction_factoryT.h"

extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    // Digitization
    app->Add(new JChainMultifactoryGeneratorT<SiliconTrackerDigi_factoryT>(
        "TOFBarrelDigiHit",
        {"TOFBarrelHits"},
        {"TOFBarrelDigiHit"},
        {
            .threshold = 0.5 * dd4hep::keV,
            .timeResolution = 0.025,    // [ns]
        },
        app
    ));

    // Convert raw digitized hits into hits with geometry info (ready for tracking)
    app->Add(new JChainMultifactoryGeneratorT<TrackerHitReconstruction_factoryT>(
        "TOFBarrelRecHit",
        {"TOFBarrelDigiHit"},    // Input data collection tags
        {"TOFBarrelRecHit"},     // Output data tag
        {
            .timeResolution = 10,
        },
        app
    ));         // Hit reco default config for factories

}
} // extern "C"
