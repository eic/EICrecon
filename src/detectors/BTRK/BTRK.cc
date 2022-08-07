// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>

#include "algorithms/digi/RawTrackerHit.h"
#include <algorithms/trk_chain_example/v2/JFactoryT_SimTrackerHitsCollection.h>
#include <algorithms/trk_chain_example/v2/JFactoryT_SiliconTrackerDigi.h>
#include <algorithms/trk_chain_example/v2/JFactoryT_TrackerHitReconstruction.h>

#include "BarrelTrackerSimHit.h"

extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);

        // Hits collection
        app->Add(new JFactoryGeneratorT<JFactoryT_SimTrackerHitsCollection<BarrelTrackerSimHit>>());

        // Digitisation
    }
}
    
