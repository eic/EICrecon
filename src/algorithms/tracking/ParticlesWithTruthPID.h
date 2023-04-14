// Original licence header: SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten, Wouter Deconinck, Dmitry Romanov


#pragma once

#include <algorithm>
#include <cmath>
#include <memory>

#include <spdlog/spdlog.h>

#include <edm4eic/TrackParametersCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>

#include <algorithms/interfaces/WithPodConfig.h>
#include "ParticlesWithTruthPIDConfig.h"


namespace eicrecon {

    using ParticlesWithAssociationNew = std::pair<std::unique_ptr<edm4eic::ReconstructedParticleCollection>, std::unique_ptr<edm4eic::MCRecoParticleAssociationCollection>>;

    class ParticlesWithTruthPID : public WithPodConfig<ParticlesWithTruthPIDConfig> {

    public:

        void init(std::shared_ptr<spdlog::logger> logger);

        ParticlesWithAssociationNew process(
                const edm4hep::MCParticleCollection* mc_particles,
                const edm4eic::TrackParametersCollection* track_params);

    private:

        std::shared_ptr<spdlog::logger> m_log;

        void tracePhiToleranceOnce(const double sinPhiOver2Tolerance, double phiTolerance);
    };
}
