
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
    app->Add(new JChainFactoryGeneratorT<SiliconTrackerDigi_factory>({"B0TrackerHits"}, "B0TrackerRawHits"));

    // Convert raw digitized hits into hits with geometry info (ready for tracking)
    app->Add(new JChainFactoryGeneratorT<TrackerHitReconstruction_factory>({"B0TrackerRawHits"},"B0TrackerRecHits"));

}
} // extern "C"

