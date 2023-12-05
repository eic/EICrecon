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
    app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
        "MPGDBarrelRecHits",
        {"MPGDBarrelRawHits"},     // Input data collection tags
        {"MPGDBarrelRecHits"},     // Output data tag
        {
            .timeResolution = 10,
        },
        app
    ));

    // Digitization
    app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
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
    app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
        "MPGDDIRCRecHits",
        {"MPGDDIRCRawHits"},    // Input data collection tags
        {"MPGDDIRCRecHits"},    // Output data tag
        {
            .timeResolution = 10,
        },
        app
    ));


    // Digitization
    app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
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
    app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
        "OuterMPGDBarrelRecHits",
        {"OuterMPGDBarrelRawHits"},     // Input data collection tags
        {"OuterMPGDBarrelRecHits"},     // Output data tag
        {
            .timeResolution = 10,
        },
        app
    ));

    // Digitization
    app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
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
    app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
        "BackwardMPGDEndcapRecHits",
        {"BackwardMPGDEndcapRawHits"},     // Input data collection tags
        {"BackwardMPGDEndcapRecHits"},     // Output data tag
        {
            .timeResolution = 10,
        },
        app
    ));

    // Digitization
    app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
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
    app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
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
