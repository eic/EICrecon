// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <string>

#include "algorithms/interfaces/WithPodConfig.h"
#include "extensions/jana/JChainMultifactoryGeneratorT.h"
#include "factories/digi/SiliconTrackerDigi_factoryT.h"
#include "factories/tracking/TrackerHitReconstruction_factoryT.h"

extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    // Digitization
    app->Add(new JChainMultifactoryGeneratorT<SiliconTrackerDigi_factoryT>(
        "MPGDBarrelRawHits",
        {"MPGDBarrelHits"},
        {"MPGDBarrelRawHits"},
        {
            .threshold = 0.25 * dd4hep::keV,
            .timeResolution = 10,
        },
        app
    ));

    // Convert raw digitized hits into hits with geometry info (ready for tracking)
    app->Add(new JChainMultifactoryGeneratorT<TrackerHitReconstruction_factoryT>(
        "MPGDBarrelRecHits",
        {"MPGDBarrelRawHits"},     // Input data collection tags
        {"MPGDBarrelRecHits"},     // Output data tag
        {
            .timeResolution = 10,
        },
        app
    ));

    // Digitization
    app->Add(new JChainMultifactoryGeneratorT<SiliconTrackerDigi_factoryT>(
        "MPGDDIRCRawHits",
        {"MPGDDIRCHits"},
        {"MPGDDIRCRawHits"},
        {
            .threshold = 0.25 * dd4hep::keV,
            .timeResolution = 10,
        },
        app
    ));

    // Convert raw digitized hits into hits with geometry info (ready for tracking)
    app->Add(new JChainMultifactoryGeneratorT<TrackerHitReconstruction_factoryT>(
        "MPGDDIRCRecHits",
        {"MPGDDIRCRawHits"},    // Input data collection tags
        {"MPGDDIRCRecHits"},    // Output data tag
        {
            .timeResolution = 10,
        },
        app
    ));


    // Digitization
    app->Add(new JChainMultifactoryGeneratorT<SiliconTrackerDigi_factoryT>(
        "OuterMPGDBarrelRawHits",
        {"OuterMPGDBarrelHits"},
        {"OuterMPGDBarrelRawHits"},
        {
            .threshold = 0.25 * dd4hep::keV,
            .timeResolution = 10,
        },
        app
    ));

    // Convert raw digitized hits into hits with geometry info (ready for tracking)
    app->Add(new JChainMultifactoryGeneratorT<TrackerHitReconstruction_factoryT>(
        "OuterMPGDBarrelRecHits",
        {"OuterMPGDBarrelRawHits"},     // Input data collection tags
        {"OuterMPGDBarrelRecHits"},     // Output data tag
        {
            .timeResolution = 10,
        },
        app
    ));

    // Digitization
    app->Add(new JChainMultifactoryGeneratorT<SiliconTrackerDigi_factoryT>(
        "BackwardMPGDEndcapRawHits",
        {"BackwardMPGDEndcapHits"},
        {"BackwardMPGDEndcapRawHits"},
        {
            .threshold = 0.25 * dd4hep::keV,
            .timeResolution = 10,
        },
        app
    ));

    // Convert raw digitized hits into hits with geometry info (ready for tracking)
    app->Add(new JChainMultifactoryGeneratorT<TrackerHitReconstruction_factoryT>(
        "BackwardMPGDEndcapRecHits",
        {"BackwardMPGDEndcapRawHits"},     // Input data collection tags
        {"BackwardMPGDEndcapRecHits"},     // Output data tag
        {
            .timeResolution = 10,
        },
        app
    ));

    // Digitization
    app->Add(new JChainMultifactoryGeneratorT<SiliconTrackerDigi_factoryT>(
        "ForwardMPGDEndcapRawHits",
        {"ForwardMPGDEndcapHits"},
        {"ForwardMPGDEndcapRawHits"},
        {
            .threshold = 0.25 * dd4hep::keV,
            .timeResolution = 10,
        },
        app
    ));

    // Convert raw digitized hits into hits with geometry info (ready for tracking)
    app->Add(new JChainMultifactoryGeneratorT<TrackerHitReconstruction_factoryT>(
        "ForwardMPGDEndcapRecHits",
        {"ForwardMPGDEndcapRawHits"},     // Input data collection tags
        {"ForwardMPGDEndcapRecHits"},     // Output data tag
        {
            .timeResolution = 10,
        },
        app
    ));

}
} // extern "C"
