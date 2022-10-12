// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <extensions/jana/JChainFactoryGeneratorT.h>
#include <algorithms/reco/MC2SmearedParticleConfig.h>

#include "MC2SmearedParticle_factory.h"
#include "MatchClusters_factory.h"

//
extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    MC2SmearedParticleConfig smearing_default_config {0};  // No momentum smearing by default

    app->Add(new JChainFactoryGeneratorT<MC2SmearedParticle_factory>(
            {"MCParticles"}, "GeneratedParticles", smearing_default_config));

    app->Add(new JChainFactoryGeneratorT<MatchClusters_factory>(
            {"EcalBarrelClusters"}, "ReconstructedParticles"));

}
} // extern "C"

