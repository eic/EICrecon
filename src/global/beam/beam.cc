// Copyright 2024, Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <string>
#include <fmt/format.h>

#include "algorithms/interfaces/WithPodConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/meta/SubDivideCollection_factory.h"
#include "algorithms/meta/SubDivideFunctors.h"
#include <edm4hep/MCParticleCollection.h>

extern "C" {
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    // Divide MCParticle collection based on generator status and PDG
    std::vector<std::string> outCollections{"BeamElectrons","BeamProtons"};
    std::vector<std::vector<int>> values{{4,11},{4,2212}};

    app->Add(new JOmniFactoryGeneratorT<SubDivideCollection_factory<edm4hep::MCParticle>>(
        "BeamParticles",
        {"MCParticles"},
        outCollections,
        {
          .function = ValueSplit<&edm4hep::MCParticle::getGeneratorStatus,&edm4hep::MCParticle::getPDG>{values},
        },
        app
      )
    );

  }
}
