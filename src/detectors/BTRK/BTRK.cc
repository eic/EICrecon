
#include "Acts/TrackFinding/CombinatorialKalmanFilter.hpp"

// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>


#include <edm4eic/RawTrackerHit.h>

#include <algorithms/digi/SiliconTrackerDigi_factory.h>
#include <algorithms/tracking/TrackerHitReconstruction_factory.h>
#include <algorithms/tracking/TrackParamTruthInit_factory.h>
#include <algorithms/tracking/TrackerSourceLinker_factory.h>
#include <algorithms/tracking/CKFTracking_factory.h>

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

        // Source linker
        app->Add(new JChainFactoryGeneratorT<TrackerSourceLinker_factory>({"BarrelTrackerHit"}, "TrackerSourceLinkerResult"));


        app->Add(new JChainFactoryGeneratorT<TrackParamTruthInit_factory>({"MCParticles"}, ""));

        app->Add(new JChainFactoryGeneratorT<CKFTracking_factory>({"TrackerSourceLinkerResult"}, ""));
    }
}

