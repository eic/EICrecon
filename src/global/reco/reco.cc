// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>

#include "extensions/jana/JChainFactoryGeneratorT.h"
#include "extensions/jana/JChainMultifactoryGeneratorT.h"

#include "ChargedParticleSelector_factory.h"
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
#include "ReconstructedElectrons_factory.h"
#include "factories/reco/MCParticleIsolator_factory.h"

//
extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    app->Add(new JChainFactoryGeneratorT<MC2SmearedParticle_factory>(
            {"MCParticles"}, "GeneratedParticles"));

    app->Add(new JChainMultifactoryGeneratorT<MatchClusters_factory>(
        "ReconstructedParticlesWithAssoc",
        { "EcalEndcapNClusters",
          "EcalBarrelScFiClusters",
          "EcalEndcapPClusters",
        },
        { "ReconstructedParticles",           // edm4eic::ReconstructedParticle
          "ReconstructedParticleAssociations" // edm4eic::MCRecoParticleAssociation
        },
        app
    ));


    app->Add(new JChainMultifactoryGeneratorT<InclusiveKinematicsElectron_factory>(
        "InclusiveKinematicsElectron",
        {
          "MCParticles",
          "ReconstructedChargedParticles",
          "ReconstructedChargedParticleAssociations"
        },
        {
          "InclusiveKinematicsElectron"
        },
        app
    ));

    app->Add(new JChainMultifactoryGeneratorT<InclusiveKinematicsTruth_factory>(
        "InclusiveKinematicsTruth",
        {
          "MCParticles",
          "ReconstructedChargedParticles",
          "ReconstructedChargedParticleAssociations"
        },
        {
          "InclusiveKinematicsTruth"
        },
        app
    ));

    app->Add(new JChainMultifactoryGeneratorT<InclusiveKinematicsJB_factory>(
        "InclusiveKinematicsJB",
        {
          "MCParticles",
          "ReconstructedChargedParticles",
          "ReconstructedChargedParticleAssociations"
        },
        {
          "InclusiveKinematicsJB"
        },
        app
    ));

    app->Add(new JChainMultifactoryGeneratorT<InclusiveKinematicsDA_factory>(
        "InclusiveKinematicsDA",
        {
          "MCParticles",
          "ReconstructedChargedParticles",
          "ReconstructedChargedParticleAssociations"
        },
        {
          "InclusiveKinematicsDA"
        },
        app
    ));

    app->Add(new JChainMultifactoryGeneratorT<InclusiveKinematicseSigma_factory>(
        "InclusiveKinematicseSigma",
        {
          "MCParticles",
          "ReconstructedChargedParticles",
          "ReconstructedChargedParticleAssociations"
        },
        {
          "InclusiveKinematicseSigma"
        },
        app
    ));

    app->Add(new JChainMultifactoryGeneratorT<InclusiveKinematicsSigma_factory>(
        "InclusiveKinematicsSigma",
        {
          "MCParticles",
          "ReconstructedChargedParticles",
          "ReconstructedChargedParticleAssociations"
        },
        {
          "InclusiveKinematicsSigma"
        },
        app
    ));

    app->Add(new JChainFactoryGeneratorT<ReconstructedElectrons_factory>(
        {"MCParticles", "ReconstructedChargedParticles", "ReconstructedChargedParticleAssociations",
        "EcalBarrelScFiClusterAssociations",
        "EcalEndcapNClusterAssociations",
        "EcalEndcapPClusterAssociations",
        "EcalEndcapPInsertClusterAssociations",
        "EcalLumiSpecClusterAssociations",
        },
        "ReconstructedElectrons"
    ));

    app->Add(new JChainMultifactoryGeneratorT<GeneratedJets_factory>(
            "GeneratedJets",
            {"MCParticles"},
            {"GeneratedJets"},
            app
    ));

    app->Add(new JChainMultifactoryGeneratorT<ReconstructedJets_factory>(
            "ReconstructedJets",
            {"ReconstructedParticles"},
            {"ReconstructedJets"},
            app
    ));

    app->Add(new JChainMultifactoryGeneratorT<ChargedParticleSelector_factory>(
            "MCChargedParticles",
            {"MCParticles"},
            {"MCChargedParticles"},
            app
    ));

    app->Add(new JChainMultifactoryGeneratorT<GeneratedJets_factory>(
            "GeneratedChargedJets",
            {"MCChargedParticles"},
            {"GeneratedChargedJets"},
            app
    ));

    app->Add(new JChainMultifactoryGeneratorT<ReconstructedJets_factory>(
            "ReconstructedChargedJets",
            {"ReconstructedChargedParticles"},
            {"ReconstructedChargedJets"},
            app
    ));

    app->Add(new JChainMultifactoryGeneratorT<MCParticleIsolator_factory>(
            "MCBeamElectrons",
            {"MCParticles"},
            {"MCBeamElectrons"},
            {
              .genStatus = 4,
              .pdg       = 11,
            },
            app
    ));

    app->Add(new JChainMultifactoryGeneratorT<MCParticleIsolator_factory>(
            "MCBeamIons",
            {"MCParticles"},
            {"MCBeamIons"},
            {
              .genStatus = 4,
              .pdg       = 2212,
              .abovePDG  = true,
            },
            app
    ));

    app->Add(new JChainMultifactoryGeneratorT<MCParticleIsolator_factory>(
            "MCPrimaryElectrons",
            {"MCParticles"},
            {"MCPrimaryElectrons"},
            {
              .genStatus = 1,
              .pdg       = 11,
            },
            app
    ));

}
} // extern "C"
