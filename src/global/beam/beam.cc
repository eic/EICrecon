// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Wouter Deconinck

#include <JANA/JApplication.h>
#include <edm4hep/MCParticleCollection.h>
#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/meta/SubDivideFunctors.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/meta/SubDivideCollection_factory.h"

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
