// Original licence header: SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023, Sylvester Joosten, Wouter Deconinck, Dmitry Romanov, Christopher Dilks


#pragma once

#include <edm4eic/CherenkovParticleIDCollection.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/TrajectoryCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/ParticleIDCollection.h>
#include <spdlog/logger.h>
#include <memory>
#include <vector>

#include "ParticlesWithPIDConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"


namespace eicrecon {

    struct ParticlesWithAssociation {
        std::unique_ptr<edm4eic::ReconstructedParticleCollection>     parts;
        std::unique_ptr<edm4eic::MCRecoParticleAssociationCollection> assocs;
        std::unique_ptr<edm4hep::ParticleIDCollection>                pids;
    };

    class ParticlesWithPID : public WithPodConfig<ParticlesWithPIDConfig> {

    public:

        void init(std::shared_ptr<spdlog::logger> logger);

        ParticlesWithAssociation process(
                const edm4hep::MCParticleCollection* mc_particles,
                const edm4eic::TrajectoryCollection* track_params,
                std::vector<const edm4eic::CherenkovParticleIDCollection*> cherenkov_pid_collections
                );

    private:

        std::shared_ptr<spdlog::logger> m_log;

        void tracePhiToleranceOnce(const double sinPhiOver2Tolerance, double phiTolerance);

        bool linkCherenkovPID(
                edm4eic::MutableReconstructedParticle& in_part,
                const edm4eic::CherenkovParticleIDCollection& in_pids,
                edm4hep::ParticleIDCollection& out_pids
                );
    };
}
