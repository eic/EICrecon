// Original licence header: SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023, Sylvester Joosten, Wouter Deconinck, Dmitry Romanov, Christopher Dilks


#pragma once

#include <algorithm>
#include <cmath>
#include <memory>
#include <cstddef>

#include <spdlog/spdlog.h>

#include <edm4eic/TrackParametersCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/CherenkovParticleIDCollection.h>
#include <edm4hep/ParticleIDCollection.h>

#include "algorithms/interfaces/WithPodConfig.h"
#include "ParticlesWithPIDConfig.h"
#include "ConvertParticleID.h"
#include "Tools.h"


namespace eicrecon {

    struct ParticlesWithAssociationNew {
        std::unique_ptr<edm4eic::ReconstructedParticleCollection>     parts;
        std::unique_ptr<edm4eic::MCRecoParticleAssociationCollection> assocs;
        std::unique_ptr<edm4hep::ParticleIDCollection>                pids;
    };

    class ParticlesWithPID : public WithPodConfig<ParticlesWithPIDConfig> {

    public:

        void init(std::shared_ptr<spdlog::logger> logger);

        ParticlesWithAssociationNew process(
                const edm4hep::MCParticleCollection* mc_particles,
                const edm4eic::TrackParametersCollection* track_params,
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
