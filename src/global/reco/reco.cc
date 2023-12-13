// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <algorithm>
#include <string>

#include "ChargedParticleSelector_factory.h"
#include "GeneratedJets_factory.h"
#include "InclusiveKinematicsDA_factory.h"
#include "InclusiveKinematicsElectron_factory.h"
#include "InclusiveKinematicsJB_factory.h"
#include "InclusiveKinematicsSigma_factory.h"
#include "InclusiveKinematicsTruth_factory.h"
#include "InclusiveKinematicseSigma_factory.h"
#include "MC2SmearedParticle_factory.h"
#include "MatchClusters_factory.h"
#include "ReconstructedElectrons_factory.h"
#include "ReconstructedJets_factory.h"
#include "algorithms/reco/ChargedParticleSelector.h"
#include "extensions/jana/JChainMultifactoryGeneratorT.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "ParticleFlow_factory.h"

//
extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    app->Add(new JOmniFactoryGeneratorT<MC2SmearedParticle_factory>(
            "GeneratedParticles",
            {"MCParticles"},
            {"GeneratedParticles"},
            app
            ));

    app->Add(new JChainMultifactoryGeneratorT<MatchClusters_factory>(
        "ReconstructedParticlesWithAssoc",
<<<<<<< HEAD
        {
          "MCParticles",
          "ReconstructedChargedParticles",
          "ReconstructedChargedParticleAssociations",
          "EcalEndcapNClusters",
          "EcalEndcapNClusterAssociations",
          "EcalBarrelScFiClusters",
          "EcalBarrelScFiClusterAssociations",
          "EcalEndcapPClusters",
          "EcalEndcapPClusterAssociations"
        },
        { "ReconstructedParticles",           // edm4eic::ReconstructedParticle
          "ReconstructedParticleAssociations" // edm4eic::MCRecoParticleAssociation
        },
        app
    ));

    app->Add(new JOmniFactoryGeneratorT<InclusiveKinematicsElectron_factory>(
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

    app->Add(new JOmniFactoryGeneratorT<InclusiveKinematicsTruth_factory>(
        "InclusiveKinematicsTruth",
        {
          "MCParticles"
        },
        {
          "InclusiveKinematicsTruth"
        },
        app
    ));

    app->Add(new JOmniFactoryGeneratorT<InclusiveKinematicsJB_factory>(
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

    app->Add(new JOmniFactoryGeneratorT<InclusiveKinematicsDA_factory>(
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

    app->Add(new JOmniFactoryGeneratorT<InclusiveKinematicseSigma_factory>(
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

    app->Add(new JOmniFactoryGeneratorT<InclusiveKinematicsSigma_factory>(
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

    app->Add(new JChainMultifactoryGeneratorT<ReconstructedElectrons_factory>(
        "ReconstructedElectrons",
        {"MCParticles", "ReconstructedChargedParticles", "ReconstructedChargedParticleAssociations",
        "EcalBarrelScFiClusterAssociations",
        "EcalEndcapNClusterAssociations",
        "EcalEndcapPClusterAssociations",
        "EcalEndcapPInsertClusterAssociations",
        "EcalLumiSpecClusterAssociations",
        },
        {"ReconstructedElectrons"},
        app
    ));

    app->Add(new JChainMultifactoryGeneratorT<GeneratedJets_factory>(
            "GeneratedJets",
            {"MCParticles"},
            {"GeneratedJets"},
            {},
            app
    ));

    app->Add(new JChainMultifactoryGeneratorT<ReconstructedJets_factory>(
            "ReconstructedJets",
            {"ReconstructedParticles"},
            {"ReconstructedJets"},
            {},
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
            {},
            app
    ));

    app->Add(new JChainMultifactoryGeneratorT<ReconstructedJets_factory>(
            "ReconstructedChargedJets",
            {"ReconstructedChargedParticles"},
            {"ReconstructedChargedJets"},
            {},
            app
    ));

    app->Add(new JChainMultifactoryGeneratorT<ParticleFlow_factory>(
        "ParticleFlow",
        {
          "ReconstructedChargedParticles",
          "CalorimeterTrackProjections",
          "EcalEndcapNClusters",
          "HcalEndcapNClusters",
          "EcalBarrelScFiClusters",
          "HcalBarrelClusters",
          "EcalEndcapPClusters",
          "LFHCALClusters"
        },
        {"ParticleFlowObjects"},
        {},
        app
    ));

}
} // extern "C"
