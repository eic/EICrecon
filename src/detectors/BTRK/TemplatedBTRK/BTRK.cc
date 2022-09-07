// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>

#include <eicd/RawTrackerHit.h>

#include <algorithms/digi/SiliconTrackerDigi_factoryT.h>
#include <algorithms/tracking/TrackerHitReconstruction_factoryT.h>

#include "BarrelTrackerSimHit.h"
#include "BarrelTrackerRawHit.h"

extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);

        // Hits collector
        //app->Add(new JFactoryGeneratorT<TrackerHitReconstruction_factoryT<BarrelTrackerSimHit>>());

        // Digitisation
        app->Add(new JFactoryGeneratorT<SiliconTrackerDigi_factoryT<BarrelTrackerSimHit, BarrelTrackerRawHit>>());

        // app->Add(new EicFactoryGeneratorT<Factory>(output_tag);

    }
}
    
