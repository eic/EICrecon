// Copyright 2024, Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JEventLevel.h>
#include <JANA/Utils/JTypeInfo.h>
#include <edm4hep/MCParticleCollection.h>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "algorithms/meta/SubDivideFunctors.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/meta/CollectionCollector_factory.h"
#include "factories/meta/SubDivideCollection_factory.h"

extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);

  using namespace eicrecon;
  using eicrecon::JOmniFactoryGeneratorT;

  // Divide MCParticle collection based on generator status and PDG
  std::vector<std::string> outCollections{"MCBeamElectrons",    "MCBeamProtons",
                                          "MCBeamNeutrons",     "MCScatteredElectrons",
                                          "MCScatteredProtons", "MCScatteredNeutrons"};
  std::vector<std::vector<int>> values{{4, 11}, {4, 2212}, {4, 2112},
                                       {1, 11}, {1, 2212}, {1, 2112}};

#if (JANA_VERSION_MAJOR > 2) || (JANA_VERSION_MAJOR == 2 && JANA_VERSION_MINOR > 4) ||             \
    (JANA_VERSION_MAJOR == 2 && JANA_VERSION_MINOR == 4 && JANA_VERSION_PATCH >= 3)
  app->Add(new JOmniFactoryGeneratorT<SubDivideCollection_factory<edm4hep::MCParticle>>(
      {.tag                   = "BeamParticles",
       .input_names           = {"MCParticles"},
       .variadic_output_names = {outCollections},
       .configs               = {
                         .function =
               ValueSplit<&edm4hep::MCParticle::getGeneratorStatus, &edm4hep::MCParticle::getPDG>{
                   values},
       }}));

  // Combine beam protons and neutrons into beam hadrons
  app->Add(new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4hep::MCParticle, true>>(
      {.tag                  = "MCBeamHadrons",
       .variadic_input_names = {{"MCBeamProtons", "MCBeamNeutrons"}},
       .output_names         = {"MCBeamHadrons"}}));

#else

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
#endif
}
}
