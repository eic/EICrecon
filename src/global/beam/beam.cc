// Copyright 2024, Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <edm4hep/MCParticleCollection.h>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "algorithms/meta/SubDivideFunctors.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/meta/Cloner_factory.h"
#include "factories/meta/CollectionCollector_factory.h"
#include "factories/meta/FirstAvailableCollection_factory.h"
#include "factories/meta/SubDivideCollection_factory.h"

extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);

  using namespace eicrecon;

  // Divide MCParticle collection based on generator status and PDG
  // Note: beam electrons and hadrons derived from MCParticles are intermediate collections;
  // MCBeamElectrons and MCBeamHadrons are defined below with a fallback to stored copies.
  std::vector<std::string> outCollections{
      "MCBeamElectronsFromMCParticles", "MCBeamProtons",      "MCBeamNeutrons",
      "MCScatteredElectrons",           "MCScatteredProtons", "MCScatteredNeutrons"};
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
  // (not optional, so that this fails when MCParticles is unavailable)
  app->Add(new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4hep::MCParticle>>(
      "MCBeamHadronsFromMCParticles", {"MCBeamProtons", "MCBeamNeutrons"},
      {"MCBeamHadronsFromMCParticles"}, app));

  // Clone beam particles into standalone collections. The electron and hadron clones
  // drop relations to other MCParticles, so they can be stored for later stages without
  // the full MCParticles collection (e.g. two-stage digitization and reconstruction).
  app->Add(new JOmniFactoryGeneratorT<Cloner_factory<edm4hep::MCParticle>>(
      "MCBeamElectronsCloned", {"MCBeamElectronsFromMCParticles"}, {"MCBeamElectronsCloned"},
      {.cloneRelations = false}, app));
  app->Add(new JOmniFactoryGeneratorT<Cloner_factory<edm4hep::MCParticle>>(
      "MCBeamProtonsCloned", {"MCBeamProtons"}, {"MCBeamProtonsCloned"}, app));
  app->Add(new JOmniFactoryGeneratorT<Cloner_factory<edm4hep::MCParticle>>(
      "MCBeamHadronsCloned", {"MCBeamHadronsFromMCParticles"}, {"MCBeamHadronsCloned"},
      {.cloneRelations = false}, app));

  // Beam particles for downstream use: from MCParticles when available, otherwise from
  // stored clones (e.g. when reconstructing from digitization-only files)
  app->Add(new JOmniFactoryGeneratorT<FirstAvailableCollection_factory<edm4hep::MCParticle>>(
      "MCBeamElectrons", {"MCBeamElectronsFromMCParticles", "MCBeamElectronsCloned"},
      {"MCBeamElectrons"}, app));
  app->Add(new JOmniFactoryGeneratorT<FirstAvailableCollection_factory<edm4hep::MCParticle>>(
      "MCBeamHadrons", {"MCBeamHadronsFromMCParticles", "MCBeamHadronsCloned"}, {"MCBeamHadrons"},
      app));
}
}
