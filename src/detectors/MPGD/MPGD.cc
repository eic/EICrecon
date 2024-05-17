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
    auto MPGDBarrelTimeResolution    =  20.0 * dd4hep::ns; // 1 / (50 MHz)
    auto MPGDBarrelIntegrationWindow = 500.0 * dd4hep::ns; // shaping time
    app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
        "MPGDBarrelRawHits",
        {
          "MPGDBarrelHits"
        },
        {
          "MPGDBarrelRawHits",
          "MPGDBarrelHitAssociations"
        },
        {
            .threshold = 0.25 * dd4hep::keV,
            .timeResolution = MPGDBarrelTimeResolution,
            .integrationWindow = MPGDBarrelIntegrationWindow,
        },
        app
    ));

    // Convert raw digitized hits into hits with geometry info (ready for tracking)
    app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
        "MPGDBarrelRecHits",
        {"MPGDBarrelRawHits"},     // Input data collection tags
        {"MPGDBarrelRecHits"},     // Output data tag
        {
            .timeResolution = MPGDBarrelTimeResolution,
        },
        app
    ));

    // Digitization
    auto OuterMPGDBarrelTimeResolution    =  20.0 * dd4hep::ns; // 1 / (50 MHz)
    auto OuterMPGDBarrelIntegrationWindow = 500.0 * dd4hep::ns; // shaping time
    app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
        "OuterMPGDBarrelRawHits",
        {
          "OuterMPGDBarrelHits"
        },
        {
          "OuterMPGDBarrelRawHits",
          "OuterMPGDBarrelHitAssociations"
        },
        {
            .threshold = 0.25 * dd4hep::keV,
            .timeResolution = OuterMPGDBarrelTimeResolution,
            .integrationWindow = OuterMPGDBarrelIntegrationWindow,
        },
        app
    ));

    // Convert raw digitized hits into hits with geometry info (ready for tracking)
    app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
        "OuterMPGDBarrelRecHits",
        {"OuterMPGDBarrelRawHits"},     // Input data collection tags
        {"OuterMPGDBarrelRecHits"},     // Output data tag
        {
            .timeResolution = OuterMPGDBarrelTimeResolution,
        },
        app
    ));

    // Digitization
    auto BackwardMPGDEndcapTimeResolution    =  20.0 * dd4hep::ns; // 1 / (50 MHz)
    auto BackwardMPGDEndcapIntegrationWindow = 500.0 * dd4hep::ns; // shaping time
    app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
        "BackwardMPGDEndcapRawHits",
        {
          "BackwardMPGDEndcapHits"
        },
        {
          "BackwardMPGDEndcapRawHits",
          "BackwardMPGDEndcapAssociations"
        },
        {
            .threshold = 0.25 * dd4hep::keV,
            .timeResolution = BackwardMPGDEndcapTimeResolution,
            .integrationWindow = BackwardMPGDEndcapIntegrationWindow,
        },
        app
    ));

    // Convert raw digitized hits into hits with geometry info (ready for tracking)
    app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
        "BackwardMPGDEndcapRecHits",
        {"BackwardMPGDEndcapRawHits"},     // Input data collection tags
        {"BackwardMPGDEndcapRecHits"},     // Output data tag
        {
            .timeResolution = BackwardMPGDEndcapTimeResolution,
        },
        app
    ));

    // Digitization
    auto ForwardMPGDEndcapTimeResolution    =  20.0 * dd4hep::ns; // 1 / (50 MHz)
    auto ForwardMPGDEndcapIntegrationWindow = 500.0 * dd4hep::ns; // shaping time
    app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
        "ForwardMPGDEndcapRawHits",
        {
          "ForwardMPGDEndcapHits"
        },
        {
          "ForwardMPGDEndcapRawHits",
          "ForwardMPGDHitAssociations"
        },
        {
            .threshold = 0.25 * dd4hep::keV,
            .timeResolution = ForwardMPGDEndcapTimeResolution,
            .integrationWindow = ForwardMPGDEndcapIntegrationWindow,
        },
        app
    ));

    // Convert raw digitized hits into hits with geometry info (ready for tracking)
    app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
        "ForwardMPGDEndcapRecHits",
        {"ForwardMPGDEndcapRawHits"},     // Input data collection tags
        {"ForwardMPGDEndcapRecHits"},     // Output data tag
        {
            .timeResolution = ForwardMPGDEndcapTimeResolution,
        },
        app
    ));

}
} // extern "C"
