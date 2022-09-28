// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>

#include <extensions/jana/JChainFactoryGeneratorT.h>

#include <global/digi/SiliconTrackerDigi_factory.h>
#include <global/tracking/TrackerHitReconstruction_factory.h>


extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    // Digitization
    app->Add(new JChainFactoryGeneratorT<SiliconTrackerDigi_factory>({"TrackerEndcapHits"}, "EndcapTrackerRawHit"));

    // Convert raw digitized hits into hits with geometry info (ready for tracking)
    app->Add(new JChainFactoryGeneratorT<TrackerHitReconstruction_factory>({"EndcapTrackerRawHit"}, "EndcapTrackerHit"));

}
} // extern "C"

