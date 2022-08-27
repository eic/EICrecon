// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>


#include <eicd/RawTrackerHit.h>

#include <algorithms/digi/SiliconTrackerDigi_factory.h>
#include <algorithms/tracking/TrackerHitReconstruction_factory.h>

#include "extensions/jana/JChainFactoryGeneratorT.h"
#include "extensions/jana/JChainFactoryT.h"

extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);

        using namespace eicrecon;

        // Digitization
        app->Add(new JChainFactoryGeneratorT<SiliconTrackerDigi_factory>({"TrackerBarrelHits"},"BarrelTrackerRawHit"));

        // Convert raw digitized hits into hits with geometry info (ready for tracking)
        app->Add(new JChainFactoryGeneratorT<TrackerHitReconstruction_factory>({"BarrelTrackerRawHit"}, "BarrelTrackerHit"));

        // app->Add(new EicFactoryGeneratorT<Factory>(output_tag);

    }
}
    
