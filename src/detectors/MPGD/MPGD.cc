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
#include "factories/digi/MPGDTrackerDigi_factory.h"
#include "factories/tracking/TrackerHitReconstruction_factory.h"

// ***** NOTA BENE:
// 2D-STRIP DIGITIZATION
// - Is prodided by "MPGDTrackerDigi".
// - Requires "MultiSegmentation" <readout> in geometry file.
// PIXEL DIGITIZATION
// - Is prodided by "SiliconTrackerDigi".
// TEMPORARILY PROVIDE FOR SWITCHING BACK TO PIXEL DIGITIZATION
#define Si_FACTORY_PATTERN 0 // Bit pattern: 0x1=CyMBaL, 0x2=Outer,...

extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    // Digitization
#if defined(Si_FACTORY_PATTERN) && (Si_FACTORY_PATTERN & 1) != 0
#  warning Switching back MPGDBarrel digitization to SiliconTrackerDigi
    app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
        "MPGDBarrelRawHits",
        {
          "MPGDBarrelHits"
        },
        {
          "MPGDBarrelRawHits",
          "MPGDBarrelRawHitAssociations"
        },
        {
            .threshold = 100 * dd4hep::eV,
            .timeResolution = 10,
        },
        app
    ));
#else
    app->Add(new JOmniFactoryGeneratorT<MPGDTrackerDigi_factory>(
        "MPGDBarrelRawHits",
        {
          "MPGDBarrelHits"
        },
        {
          "MPGDBarrelRawHits",
          "MPGDBarrelRawHitAssociations"
        },
        {
            .readout = "MPGDBarrelHits",
            .threshold = 100 * dd4hep::eV,
            .timeResolution = 10,
        },
        app
    ));
#endif

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
#if defined(Si_FACTORY_PATTERN) && (Si_FACTORY_PATTERN & 2) != 0
#  warning Switching back OuterMPGDBarrel digitization to SiliconTrackerDigi
   app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
        "OuterMPGDBarrelRawHits",
        {
          "OuterMPGDBarrelHits"
        },
        {
          "OuterMPGDBarrelRawHits",
          "OuterMPGDBarrelRawHitAssociations"
        },
        {
            .threshold = 100 * dd4hep::eV,
            .timeResolution = 10,
        },
        app
    ));
#else
    app->Add(new JOmniFactoryGeneratorT<MPGDTrackerDigi_factory>(
        "OuterMPGDBarrelRawHits",
        {
          "OuterMPGDBarrelHits"
        },
        {
          "OuterMPGDBarrelRawHits",
          "OuterMPGDBarrelRawHitAssociations"
        },
        {
            .readout = "OuterMPGDBarrelHits",
            .threshold = 100 * dd4hep::eV,
            .timeResolution = 10,
        },
        app
    ));
#endif

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
        {
          "BackwardMPGDEndcapHits"
        },
        {
          "BackwardMPGDEndcapRawHits",
          "BackwardMPGDEndcapRawHitAssociations"
        },
        {
            .threshold = 100 * dd4hep::eV,
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
        {
          "ForwardMPGDEndcapHits"
        },
        {
          "ForwardMPGDEndcapRawHits",
          "ForwardMPGDEndcapRawHitAssociations"
        },
        {
            .threshold = 100 * dd4hep::eV,
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
