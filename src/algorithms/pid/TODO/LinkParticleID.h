// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

// General algorithm to link ParticleID objects to reconstructed particles

#pragma once

// general
#include <algorithm>

// data model
#include <algorithms/reco/ParticlesWithAssociation.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/CherenkovParticleIDCollection.h>
#include <edm4hep/ParticleIDCollection.h>
#include <edm4hep/utils/vector_utils.h>

// DD4hep
#include <Evaluator/DD4hepUnits.h>

// EICrecon
#include "LinkParticleIDConfig.h"
#include "ConvertParticleID.h"
#include "Tools.h"
#include <algorithms/interfaces/WithPodConfig.h>
#include <spdlog/spdlog.h>

namespace eicrecon {

  // algorithm
  class LinkParticleID : public WithPodConfig<LinkParticleIDConfig> {

    public:
      LinkParticleID() = default;
      ~LinkParticleID() {}

      void AlgorithmInit(std::shared_ptr<spdlog::logger>& logger);
      void AlgorithmChangeRun();

      // AlgorithmProcess
      // - overload this function to support different collections from other PID subsystems
      std::vector<eicrecon::ParticlesWithAssociation*> AlgorithmProcess(
          std::vector<const eicrecon::ParticlesWithAssociation*>& in_particles,
          std::vector<const edm4eic::CherenkovParticleID*>&       in_pids
          );

      // LinkParticle function: link input `ReconstructedParticle` to a `ParticleID` from `in_pids`;
      // returns a modified `ReconstructedParticle` that includes the `ParticleID` relations
      edm4eic::MutableReconstructedParticle LinkParticle(
          edm4eic::ReconstructedParticle                    in_particle,
          std::vector<const edm4eic::CherenkovParticleID*>& in_pids
          );

    private:
      std::shared_ptr<spdlog::logger> m_log;

  };
}
