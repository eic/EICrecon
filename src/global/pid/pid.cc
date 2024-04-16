// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023, Christopher Dilks


#include <JANA/JApplication.h>
#include <string>

#include "algorithms/interfaces/WithPodConfig.h"
// algorithm configurations
#include "algorithms/pid/ParticlesWithPIDConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
// factories
#include "global/pid/ParticlesWithPID_factory.h"

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
    app->Add(new JOmniFactoryGeneratorT<ParticlesWithPID_factory>(
          "ChargedParticlesWithAssociations",
          {
            "MCParticles",                      // edm4hep::MCParticle
            "CentralCKFTracks",                 // edm4eic::Track
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

    app->Add(new JOmniFactoryGeneratorT<ParticlesWithPID_factory>(
          "SeededChargedParticlesWithAssociations",
          {
            "MCParticles",                      // edm4hep::MCParticle
            "CentralCKFSeededTracks",           // edm4eic::Track
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
              app->Add(new JOmniFactoryGeneratorT<ParticlesWithPID_factory>(
          "SeededFilteredChargedParticlesWithAssociations",
          {
            "MCParticles",                      // edm4hep::MCParticle
            "CentralCKFSeededTracksFiltered",           // edm4eic::Track
            "DRICHMergedIrtCherenkovParticleID" // edm4eic::CherenkovParticleID
          },
          {
            "ReconstructedSeededChargedParticlesFiltered",            // edm4eic::ReconstructedParticle
            "ReconstructedSeededChargedParticleFilteredAssociations", // edm4eic::MCRecoParticleAssociation
            "ReconstructedSeededChargedParticleFilteredIDs"           // edm4hep::ParticleID
          },
          link_cfg,
          app
          ));

  }
}
