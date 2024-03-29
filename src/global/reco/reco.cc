// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <edm4eic/Cluster.h>
#include <edm4eic/MCRecoClusterParticleAssociation.h>
#include <algorithm>
#include <map>
#include <memory>

#include "algorithms/reco/InclusiveKinematicsDA.h"
#include "algorithms/reco/InclusiveKinematicsElectron.h"
#include "algorithms/reco/InclusiveKinematicsJB.h"
#include "algorithms/reco/InclusiveKinematicsSigma.h"
#include "algorithms/reco/InclusiveKinematicseSigma.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/meta/CollectionCollector_factory.h"
#include "factories/reco/InclusiveKinematicsReconstructed_factory.h"
#include "factories/reco/InclusiveKinematicsTruth_factory.h"
#include "factories/reco/TransformBreitFrame_factory.h"
#include "global/reco/ChargedReconstructedParticleSelector_factory.h"
#include "global/reco/JetReconstruction_factory.h"
#include "global/reco/MC2SmearedParticle_factory.h"
#include "global/reco/MatchClusters_factory.h"
#include "global/reco/ReconstructedElectrons_factory.h"
#include "factories/meta/FilterByAssociations_factory.h"

//
extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    // Finds associations matched to initial scattered electrons
    app->Add(new JOmniFactoryGeneratorT<FilterByAssociations_factory<edm4hep::MCParticle,
                                                                    [](auto* obj) { return obj->getObjectID(); },
                                                                    edm4eic::MCRecoParticleAssociation, 
                                                                    [](auto* obj) { return obj->getSim().getObjectID(); }>>(
          "MCScatteredElectronAssociations",
          {"MCScatteredElectrons", "ReconstructedChargedParticleAssociations"},
          {"MCScatteredElectronAssociations","MCNotScatteredElectronAssociations"},
          app
    ));

    app->Add(new JOmniFactoryGeneratorT<MC2SmearedParticle_factory>(
            "GeneratedParticles",
            {"MCParticles"},
            {"GeneratedParticles"},
            app
            ));

    app->Add(new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::Cluster>>(
        "EcalClusters",
        {
          "EcalEndcapNClusters",
          "EcalBarrelScFiClusters",
          "EcalEndcapPClusters",
        },
        {"EcalClusters"},
        app));

    app->Add(new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::MCRecoClusterParticleAssociation>>(
        "EcalClusterAssociations",
        {
          "EcalEndcapNClusterAssociations",
          "EcalBarrelScFiClusterAssociations",
          "EcalEndcapPClusterAssociations",
        },
        {"EcalClusterAssociations"},
        app));

    app->Add(new JOmniFactoryGeneratorT<MatchClusters_factory>(
        "ReconstructedParticlesWithAssoc",
        {
          "MCParticles",
          "ReconstructedChargedParticles",
          "ReconstructedChargedParticleAssociations",
          "EcalClusters",
          "EcalClusterAssociations",
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


    app->Add(new JOmniFactoryGeneratorT<TransformBreitFrame_factory>(
            "ReconstructedBreitFrameParticles",
            {"MCParticles","InclusiveKinematicsElectron","ReconstructedParticles"},
            {"ReconstructedBreitFrameParticles"},
            {},
            app
    ));

}
} // extern "C"
