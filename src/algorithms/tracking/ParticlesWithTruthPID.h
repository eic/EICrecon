// Original licence header: SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten, Wouter Deconinck, Dmitry Romanov


#pragma once

#include <algorithm>
#include <cmath>

#include <spdlog/spdlog.h>

#include <edm4eic/TrackParameters.h>
#include <edm4hep/MCParticle.h>

#include <algorithms/reco/ParticlesWithAssociation.h>
#include <algorithms/interfaces/WithPodConfig.h>
#include "ParticlesWithTruthPIDConfig.h"



namespace eicrecon {
    class ParticlesWithTruthPID : public WithPodConfig<ParticlesWithTruthPIDConfig> {

    public:

        void init(std::shared_ptr<spdlog::logger> logger);

        ParticlesWithAssociation *process(
                const std::vector<const edm4hep::MCParticle *> &mc_particles,
                const std::vector<const edm4eic::TrackParameters*> &track_params);

    private:

        std::shared_ptr<spdlog::logger> m_log;

        void tracePhiToleranceOnce(const double sinPhiOver2Tolerance, double phiTolerance);
    };
}

