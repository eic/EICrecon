// Copyright 2024, Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <edm4hep/MCParticleCollection.h>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "algorithms/meta/SubDivideFunctors.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/meta/Cloner_factory.h"
#include "factories/meta/CollectionCollector_factory.h"
#include "factories/meta/SubDivideCollection_factory.h"

extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);

  using namespace eicrecon;

  // Divide MCParticle collection based on generator status and PDG
  // (subset collections of MCParticles)
  std::vector<std::string> outCollections{"MCBeamElectronsFromMCParticles",
                                          "MCBeamProtonsFromMCParticles",
                                          "MCBeamNeutronsFromMCParticles",
                                          "MCScatteredElectrons",
                                          "MCScatteredProtons",
                                          "MCScatteredNeutrons"};
  std::vector<std::vector<int>> values{{4, 11}, {4, 2212}, {4, 2112},
                                       {1, 11}, {1, 2212}, {1, 2112}};

  app->Add(new JOmniFactoryGeneratorT<SubDivideCollection_factory<edm4hep::MCParticle>>(
      "BeamParticles", {"MCParticles"}, outCollections,
      {
          .function =
              ValueSplit<&edm4hep::MCParticle::getGeneratorStatus, &edm4hep::MCParticle::getPDG>{
                  values},
      },
      app));

  // Combine beam protons and neutrons into beam hadrons
  app->Add(new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4hep::MCParticle, true>>(
      "MCBeamHadronsFromMCParticles",
      {"MCBeamProtonsFromMCParticles", "MCBeamNeutronsFromMCParticles"},
      {"MCBeamHadronsFromMCParticles"}, app));

  // Beam particles for downstream use: standalone clones without relations to other
  // MCParticles, so they can be written and read back without MCParticles. When reconstructing
  // from digitization-only files, these collections are read from the input file, and the
  // collections above are not used.
  app->Add(new JOmniFactoryGeneratorT<Cloner_factory<edm4hep::MCParticle>>(
      "MCBeamElectrons", {"MCBeamElectronsFromMCParticles"}, {"MCBeamElectrons"},
      {.cloneRelations = false}, app));
  app->Add(new JOmniFactoryGeneratorT<Cloner_factory<edm4hep::MCParticle>>(
      "MCBeamHadrons", {"MCBeamHadronsFromMCParticles"}, {"MCBeamHadrons"},
      {.cloneRelations = false}, app));

  // Beam protons as a standalone collection (with relations)
  app->Add(new JOmniFactoryGeneratorT<Cloner_factory<edm4hep::MCParticle>>(
      "MCBeamProtonsCloned", {"MCBeamProtonsFromMCParticles"}, {"MCBeamProtonsCloned"}, app));
}
}
