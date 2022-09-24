// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Acts/Propagator/Navigator.hpp>

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <extensions/jana/JChainFactoryGeneratorT.h>

#include "TrackerSourceLinker_factory.h"
#include "TrackParamTruthInit_factory.h"
#include "TrackingResult_factory.h"
#include "ReconstructedParticle_factory.h"
#include "TrackParameters_factory.h"
#include "CKFTracking_factory.h"




//
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

    app->Add(new JChainFactoryGeneratorT<CKFTracking_factory>(
            {"CentralTrackerSourceLinker"}, "CentralCKFTrajectories"));

    app->Add(new JChainFactoryGeneratorT<TrackingResult_factory>(
            {"CentralCKFTrajectories"}, "CentralTrackingParticles"));

    app->Add(new JChainFactoryGeneratorT<ReconstructedParticle_factory>(
            {"CentralTrackingParticles"}, "ReconstructedParticles"));

    app->Add(new JChainFactoryGeneratorT<TrackParameters_factory>(
            {"CentralTrackingParticles"}, "TrackParameters"));
}
} // extern "C"

