// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023, Christopher Dilks

#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <string>
#include <vector>

#include "extensions/jana/JOmniFactoryGeneratorT.h"
// factories
#include "factories/pid/MatchToRICHPID_factory.h"

extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);

  using namespace eicrecon;
  using jana::components::JOmniFactoryGeneratorT;

  // wiring between factories and data ///////////////////////////////////////

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
      }));
}
}
