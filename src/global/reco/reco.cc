// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <extensions/jana/JChainFactoryGeneratorT.h>
#include <extensions/jana/JChainMultifactoryGeneratorT.h>
#include <algorithms/reco/MC2SmearedParticleConfig.h>

#include "MC2SmearedParticle_factory.h"
#include "MatchClusters_factory.h"
#include "InclusiveKinematicsElectron_factory.h"
#include "InclusiveKinematicsTruth_factory.h"
#include "InclusiveKinematicsJB_factory.h"
#include "InclusiveKinematicsDA_factory.h"
#include "InclusiveKinematicseSigma_factory.h"
#include "InclusiveKinematicsSigma_factory.h"
#include "GeneratedJets_factory.h"
#include "ReconstructedJets_factory.h"

//
extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    MC2SmearedParticleConfig smearing_default_config {0};  // No momentum smearing by default

    app->Add(new JChainFactoryGeneratorT<MC2SmearedParticle_factory>(
            {"MCParticles"}, "GeneratedParticles", smearing_default_config));

    app->Add(new JChainMultifactoryGeneratorT<MatchClusters_factory>(
        "ReconstructedParticlesWithAssoc",
        { "EcalEndcapNClusters",
          "EcalBarrelSciFiClusters",
          "EcalEndcapPClusters",
        },
        { "ReconstructedParticles",           // edm4eic::ReconstructedParticle
          "ReconstructedParticleAssociations" // edm4eic::MCRecoParticleAssociation
        },
        app
    ));


    app->Add(new JChainFactoryGeneratorT<InclusiveKinematicsElectron_factory>(
            {"MCParticles", "ReconstructedChargedParticles", "ReconstructedChargedParticleAssociations"}, "InclusiveKinematicsElectron"));

    app->Add(new JChainFactoryGeneratorT<InclusiveKinematicsTruth_factory>(
            {"MCParticles", "ReconstructedChargedParticles", "ReconstructedChargedParticleAssociations"}, "InclusiveKinematicsTruth"));

    app->Add(new JChainFactoryGeneratorT<InclusiveKinematicsJB_factory>(
            {"MCParticles", "ReconstructedChargedParticles", "ReconstructedChargedParticleAssociations"}, "InclusiveKinematicsJB"));

    app->Add(new JChainFactoryGeneratorT<InclusiveKinematicsDA_factory>(
            {"MCParticles", "ReconstructedChargedParticles", "ReconstructedChargedParticleAssociations"}, "InclusiveKinematicsDA"));

    app->Add(new JChainFactoryGeneratorT<InclusiveKinematicseSigma_factory>(
            {"MCParticles", "ReconstructedChargedParticles", "ReconstructedChargedParticleAssociations"}, "InclusiveKinematicseSigma"));

    app->Add(new JChainFactoryGeneratorT<InclusiveKinematicsSigma_factory>(
            {"MCParticles", "ReconstructedChargedParticles", "ReconstructedChargedParticleAssociations"}, "InclusiveKinematicsSigma"));

    app->Add(new JChainMultifactoryGeneratorT<GeneratedJets_factory>(
             "GeneratedJets",
            {"MCParticles"},
            {"GeneratedJets"},
            app
    ));
    app->Add(new JChainMultifactoryGeneratorT<ReconstructedJets_factory>(
            "ReconstructedJets",
            {"ReconstructedChargedParticles"},
            {"ReconstructedJets"},
            app
    ));
}
} // extern "C"
