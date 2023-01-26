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
#include "ReconstructedParticles_factory.h"
#include "ReconstructedParticleAssociations_factory.h"
#include "InclusiveKinematicsElectron_factory.h"
#include "InclusiveKinematicsTruth_factory.h"
#include "InclusiveKinematicsJB_factory.h"
#include "InclusiveKinematicsDA_factory.h"
#include "InclusiveKinematicseSigma_factory.h"

//
extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    MC2SmearedParticleConfig smearing_default_config {0};  // No momentum smearing by default

    app->Add(new JChainFactoryGeneratorT<MC2SmearedParticle_factory>(
            {"MCParticles"}, "GeneratedParticles", smearing_default_config));

    app->Add(new JChainFactoryGeneratorT<MatchClusters_factory>(
            {"EcalEndcapNClusters", "EcalBarrelSciGlassClusters"}, "ReconstructedParticlesWithAssoc"));

    app->Add(new JChainFactoryGeneratorT<ReconstructedParticles_factory>(
            {"ReconstructedParticlesWithAssoc"}, "ReconstructedParticles"));

    app->Add(new JChainFactoryGeneratorT<ReconstructedParticleAssociations_factory>(
            {"ChargedParticlesWithAssociations"},
            "ReconstructedParticlesAssociations"));

    app->Add(new JChainFactoryGeneratorT<InclusiveKinematicsElectron_factory>(
            {"MCParticles", "ReconstructedParticles", "ReconstructedParticlesAssociations"}, "InclusiveKinematicsElectron"));
            
    app->Add(new JChainFactoryGeneratorT<InclusiveKinematicsTruth_factory>(
            {"MCParticles", "ReconstructedParticles", "ReconstructedParticlesAssociations"}, "InclusiveKinematicsTruth"));

    app->Add(new JChainFactoryGeneratorT<InclusiveKinematicsJB_factory>(
            {"MCParticles", "ReconstructedParticles", "ReconstructedParticlesAssociations"}, "InclusiveKinematicsJB"));

    app->Add(new JChainFactoryGeneratorT<InclusiveKinematicsDA_factory>(
            {"MCParticles", "ReconstructedParticles", "ReconstructedParticlesAssociations"}, "InclusiveKinematicsDA"));

    app->Add(new JChainFactoryGeneratorT<InclusiveKinematicseSigma_factory>(
            {"MCParticles", "ReconstructedParticles", "ReconstructedParticlesAssociations"}, "InclusiveKinematicseSigma"));

}
} // extern "C"

