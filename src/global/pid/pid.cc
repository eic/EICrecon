// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023, Christopher Dilks


#include <JANA/JApplication.h>
#include <string>

#include "algorithms/interfaces/WithPodConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
// factories
#include "global/pid/MatchToRICHPID_factory.h"

extern "C" {
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    // wiring between factories and data ///////////////////////////////////////
    // clang-format off

    // link charged particles to PID and to MC truth
    app->Add(new JOmniFactoryGeneratorT<MatchToRICHPID_factory>(
          "ChargedParticlesWithAssociations",
          {
            "ReconstructedChargedWithoutPIDParticles",            // edm4eic::ReconstructedParticle
            "ReconstructedChargedWithoutPIDParticleAssociations", // edm4eic::MCRecoParticleAssociationCollection
            "DRICHMergedIrtCherenkovParticleID",                  // edm4eic::CherenkovParticleID
          },
          {
            "ReconstructedChargedRealPIDParticles",            // edm4eic::ReconstructedParticle
            "ReconstructedChargedRealPIDParticleAssociations", // edm4eic::MCRecoParticleAssociationCollection
            "ReconstructedChargedRealPIDParticleIDs",          // edm4hep::ParticleID
          },
          app
          ));
  }
}
