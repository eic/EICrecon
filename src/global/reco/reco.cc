// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <DD4hep/DD4hepUnits.h>
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

    app->Add(new JOmniFactoryGeneratorT<ParticleFlow_factory>(
        "ParticleFlowEndcapN",
        {
          "ReconstructedChargedParticles",  // edm4eic::ReconstructedParticle
          "CalorimeterTrackProjections",    // edm4eic::TrackSegment
          "EcalEndcapNClusters",            // edm4eic::Cluster
          "HcalEndcapNClusters"             // edm4eic::Cluster
        },
        {"EndcapNParticleFlowObjects"},  // edm4eic::ReconstructedParticle
        {
          .flowAlgo = 0,
          .ecalDetName = "EcalEndcapN",
          .hcalDetName = "HcalEndcapN",
          .minTrkMomentum = 0.1 * dd4hep::GeV,
          .minECalEnergy = 0.1 * dd4hep::GeV,
          .minHCalEnergy = 0.1 * dd4hep::GeV,
          .ecalSumRadius = 1.0,
          .hcalSumRadius = 1.0,
          .ecalFracSub = 1.0,
          .hcalFracSub = 1.0
        },
        app
    ));

    app->Add(new JOmniFactoryGeneratorT<ParticleFlow_factory>(
        "ParticleFlowBarrel",
        {
          "ReconstructedChargedParticles",  // edm4eic::ReconstructedParticle
          "CalorimeterTrackProjections",    // edm4eic::TrackSegment
          "EcalBarrelScFiClusters",         // edm4eic::Cluster
          "HcalBarrelClusters"              // edm4eic::Cluster
        },
        {"BarrelParticleFlowObjects"},  // edm4eic::ReconstructedParticle
        {
          .flowAlgo = 0,
          .ecalDetName = "EcalBarrelScFi",
          .hcalDetName = "HcalBarrel",
          .minTrkMomentum = 0.1 * dd4hep::GeV,
          .minECalEnergy = 0.1 * dd4hep::GeV,
          .minHCalEnergy = 0.1 * dd4hep::GeV,
          .ecalSumRadius = 1.0,
          .hcalSumRadius = 1.0,
          .ecalFracSub = 1.0,
          .hcalFracSub = 1.0
        },
        app
    ));

    app->Add(new JOmniFactoryGeneratorT<ParticleFlow_factory>(
        "ParticleFlowEndcapP",
        {
          "ReconstructedChargedParticles",  // edm4eic::ReconstructedParticle
          "CalorimeterTrackProjections",    // edm4eic::TrackSegment
          "EcalEndcapPClusters",            // edm4eic::Cluster
          "LFHCALClusters"                  // edm4eic::Cluster
        },
        {"EndcapPParticleFlowObjects"},  // edm4eic::ReconstructedParticle
        {
          .flowAlgo = 0,
          .ecalDetName = "EcalEndcapP",
          .hcalDetName = "LFHCAL",
          .minTrkMomentum = 0.1 * dd4hep::GeV,
          .minECalEnergy = 0.1 * dd4hep::GeV,
          .minHCalEnergy = 0.1 * dd4hep::GeV,
          .ecalSumRadius = 1.0,
          .hcalSumRadius = 1.0,
          .ecalFracSub = 1.0,
          .hcalFracSub = 1.0
        },
        app
    ));

}
} // extern "C"
