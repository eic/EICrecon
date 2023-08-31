// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023, Christopher Dilks


#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include "extensions/jana/JChainMultifactoryGeneratorT.h"

// factories
#include "global/pid/ParticlesWithPID_factory.h"

// algorithm configurations
#include "algorithms/pid/ParticlesWithPIDConfig.h"

extern "C" {
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    // configuration parameters ///////////////////////////////////////////////

    // linking of reconstructed particles to PID objects
    ParticlesWithPIDConfig link_cfg;
    link_cfg.momentumRelativeTolerance = 100.0; /// Matching momentum effectively disabled
    link_cfg.phiTolerance              = 0.1; /// Matching phi tolerance [rad]
    link_cfg.etaTolerance              = 0.2; /// Matching eta tolerance


    // wiring between factories and data ///////////////////////////////////////
    // clang-format off

    // link charged particles to PID and to MC truth
    app->Add(new JChainMultifactoryGeneratorT<ParticlesWithPID_factory>(
          "ChargedParticlesWithAssociations",
          {
            "MCParticles",                      // edm4hep::MCParticle
            "CentralCKFTrajectories",           // edm4eic::Trajectory
            "DRICHMergedIrtCherenkovParticleID" // edm4eic::CherenkovParticleID
          },
          {
            "ReconstructedChargedParticles",            // edm4eic::ReconstructedParticle
            "ReconstructedChargedParticleAssociations", // edm4eic::MCRecoParticleAssociation
            "ReconstructedChargedParticleIDs"           // edm4hep::ParticleID
          },
          link_cfg,
          app
          ));

    app->Add(new JChainMultifactoryGeneratorT<ParticlesWithPID_factory>(
          "ChargedParticlesWithAssociations",
          {
            "MCParticles",                      // edm4hep::MCParticle
            "CentralCKFSeededTrajectories",     // edm4eic::Trajectory
            "DRICHMergedIrtCherenkovParticleID" // edm4eic::CherenkovParticleID
          },
          {
            "ReconstructedSeededChargedParticles",            // edm4eic::ReconstructedParticle
            "ReconstructedSeededChargedParticleAssociations", // edm4eic::MCRecoParticleAssociation
            "ReconstructedSeededChargedParticleIDs"           // edm4hep::ParticleID
          },
          link_cfg,
          app
          ));

  }
}
