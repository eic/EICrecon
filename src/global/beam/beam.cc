// Copyright 2024, Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <edm4hep/MCParticleCollection.h>
#include <fmt/core.h>
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
  std::vector<std::string> outCollections{"MCBeamElectrons",    "MCBeamProtons",
                                          "MCBeamNeutrons",     "MCScatteredElectrons",
                                          "MCScatteredProtons", "MCScatteredNeutrons"};
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
      "MCBeamHadrons", {"MCBeamProtons", "MCBeamNeutrons"}, {"MCBeamHadrons"}, app));

  // Clone MCBeamElectrons and MCBeamProtons for two-stage workflows
  // This allows storing just the beam particles without the full MCParticles collection
  app->Add(new JOmniFactoryGeneratorT<Cloner_factory<edm4hep::MCParticle>>(
      "MCBeamElectronsCloned", {"MCBeamElectrons"}, {"MCBeamElectronsCloned"}, app));
  app->Add(new JOmniFactoryGeneratorT<Cloner_factory<edm4hep::MCParticle>>(
      "MCBeamProtonsCloned", {"MCBeamProtons"}, {"MCBeamProtonsCloned"}, app));
}
}
