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
#include "TrackerReconstructedParticle_factory.h"
#include "TrackParameters_factory.h"
#include "CKFTracking_factory.h"
#include "TrackerHitCollector_factory.h"
#include "TrackParameters_factory.h"
#include "ParticlesWithTruthPID_factory.h"
#include <global/reco/ReconstructedParticles_factory.h>
#include <global/reco/ReconstructedParticleAssociations_factory.h>

//
extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    app->Add(new JChainFactoryGeneratorT<TrackParamTruthInit_factory>(
            {"MCParticles"}, "InitTrackParams"));

    // Tracker hits collector
    app->Add(new JChainFactoryGeneratorT<TrackerHitCollector_factory>(
                     {"BarrelTrackerHit",
                      "BarrelVertexHit",
                      "EndcapTrackerHit",
                      "MPGDTrackerHit",
                      "TOFEndcapTrackerHit",
                      "TOFBarrelTrackerHit"},
                     "trackerHits"));

    // Source linker
    app->Add(new JChainFactoryGeneratorT<TrackerSourceLinker_factory>(
            {"trackerHits"}, "CentralTrackerSourceLinker"));

    app->Add(new JChainFactoryGeneratorT<CKFTracking_factory>(
            {"CentralTrackerSourceLinker"}, "CentralCKFTrajectories"));

    app->Add(new JChainFactoryGeneratorT<TrackingResult_factory>(
            {"CentralCKFTrajectories"}, "CentralTrackingParticles"));

    app->Add(new JChainFactoryGeneratorT<TrackerReconstructedParticle_factory>(
            {"CentralTrackingParticles"}, "outputParticles"));

    app->Add(new JChainFactoryGeneratorT<TrackParameters_factory>(
            {"CentralTrackingParticles"}, "outputTrackParameters"));

    app->Add(new JChainFactoryGeneratorT<ParticlesWithTruthPID_factory>(
            {"MCParticles",                         // Tag for edm4hep::MCParticle
            "outputTrackParameters"},               // Tag for edm4eic::TrackParameters
            "ChargedParticlesWithAssociations"));   // eicrecon::ParticlesWithAssociation

    app->Add(new JChainFactoryGeneratorT<ReconstructedParticles_factory>(
            {"ChargedParticlesWithAssociations"},
            "ReconstructedChargedParticles"));

    app->Add(new JChainFactoryGeneratorT<ReconstructedParticleAssociations_factory>(
            {"ChargedParticlesWithAssociations"},
            "ReconstructedChargedParticlesAssociations"));


}
} // extern "C"

