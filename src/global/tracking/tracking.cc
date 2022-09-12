// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <extensions/jana/JChainFactoryGeneratorT.h>
#include <Acts/Propagator/Navigator.hpp>
#include "TrackerSourceLinker_factory.h"
#include "TrackParamTruthInit_factory.h"
#include "CKFTracking_factory.h"


extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;


    app->Add(new JChainFactoryGeneratorT<TrackParamTruthInit_factory>(
            {"MCParticles"}, "InitTrackParams"));

    // Source linker
    app->Add(new JChainFactoryGeneratorT<TrackerSourceLinker_factory>(
            {"BarrelTrackerHit",
             "BarrelVertexHit",
             "EndcapTrackerHit",
             "MPGDTrackerHit"   },
            "CentralTrackerSourceLinker"));

    // tracking
    app->Add(new JChainFactoryGeneratorT<CKFTracking_factory>(
            {"CentralTrackerSourceLinker"}, "Trajectories"));
}
} // extern "C"

