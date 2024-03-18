// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <algorithm>
#include <string>

#include "algorithms/reco/InclusiveKinematicsDA.h"
#include "algorithms/reco/InclusiveKinematicsElectron.h"
#include "algorithms/reco/InclusiveKinematicsJB.h"
#include "algorithms/reco/InclusiveKinematicsSigma.h"
#include "algorithms/reco/InclusiveKinematicsTruth.h"
#include "algorithms/reco/InclusiveKinematicseSigma.h"

#include "factories/reco/InclusiveKinematicsReconstructed_factory.h"
#include "factories/reco/InclusiveKinematicsTruth_factory.h"

#include "global/reco/ChargedReconstructedParticleSelector_factory.h"
#include "global/reco/JetReconstruction_factory.h"
#include "global/reco/MC2SmearedParticle_factory.h"
#include "global/reco/MatchClusters_factory.h"
#include "global/reco/ReconstructedElectrons_factory.h"
#include "extensions/jana/JChainMultifactoryGeneratorT.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "extensions/spdlog/SpdlogExtensions.h"

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


    app->Add(new JOmniFactoryGeneratorT<InclusiveKinematicsReconstructed_factory<InclusiveKinematicsElectron>>(
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

    app->Add(new JOmniFactoryGeneratorT<InclusiveKinematicsReconstructed_factory<InclusiveKinematicsJB>>(
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

    app->Add(new JOmniFactoryGeneratorT<InclusiveKinematicsReconstructed_factory<InclusiveKinematicsDA>>(
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

    app->Add(new JOmniFactoryGeneratorT<InclusiveKinematicsReconstructed_factory<InclusiveKinematicseSigma>>(
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

    app->Add(new JOmniFactoryGeneratorT<InclusiveKinematicsReconstructed_factory<InclusiveKinematicsSigma>>(
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

    app->Add(new JOmniFactoryGeneratorT<ReconstructedElectrons_factory>(
        "ReconstructedElectrons",
        {"MCParticles", "ReconstructedChargedParticles", "ReconstructedChargedParticleAssociations",
        "EcalBarrelScFiClusterAssociations",
        "EcalEndcapNClusterAssociations",
        "EcalEndcapPClusterAssociations",
        "EcalEndcapPInsertClusterAssociations",
        "EcalLumiSpecClusterAssociations",
        },
        {"ReconstructedElectrons"},
        {},
        app
    ));

    app->Add(new JOmniFactoryGeneratorT<JetReconstruction_factory>(
            "GeneratedJets",
            {"GeneratedParticles"},
            {"GeneratedJets"},
            {},
            app
    ));

    app->Add(new JOmniFactoryGeneratorT<JetReconstruction_factory>(
            "ReconstructedJets",
            {"ReconstructedParticles"},
            {"ReconstructedJets"},
            {},
            app
    ));

    app->Add(new JOmniFactoryGeneratorT<ChargedReconstructedParticleSelector_factory>(
            "GeneratedChargedParticles",
            {"GeneratedParticles"},
            {"GeneratedChargedParticles"},
            app
    ));

    app->Add(new JOmniFactoryGeneratorT<JetReconstruction_factory>(
            "GeneratedChargedJets",
            {"GeneratedChargedParticles"},
            {"GeneratedChargedJets"},
            {},
            app
    ));

    app->Add(new JOmniFactoryGeneratorT<JetReconstruction_factory>(
            "ReconstructedChargedJets",
            {"ReconstructedChargedParticles"},
            {"ReconstructedChargedJets"},
            {},
            app
    ));

}
} // extern "C"
