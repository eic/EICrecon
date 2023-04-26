// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

// General algorithm to link ParticleID objects to reconstructed particles

#pragma once

// general
#include <algorithm>
#include <cstddef>

// data model
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
      // - overloaded, depending on input particle type
      std::unique_ptr<edm4eic::ReconstructedParticleCollection> AlgorithmProcess(
          const edm4eic::CherenkovParticleIDCollection*   in_pids,
          const edm4eic::ReconstructedParticleCollection* in_particles
          );
      std::unique_ptr<edm4eic::MCRecoParticleAssociationCollection> AlgorithmProcess(
          const edm4eic::CherenkovParticleIDCollection*       in_pids,
          const edm4eic::MCRecoParticleAssociationCollection* in_assocs
          );

      // LinkParticle function: link input `ReconstructedParticle` to a `ParticleID` from `in_pids`;
      // returns a modified `ReconstructedParticle` that includes the `ParticleID` relations
      edm4eic::MutableReconstructedParticle LinkParticle(
          const edm4eic::CherenkovParticleIDCollection* in_pids,
          edm4eic::ReconstructedParticle                in_particle
          );

    private:
      std::shared_ptr<spdlog::logger> m_log;

  };
}
