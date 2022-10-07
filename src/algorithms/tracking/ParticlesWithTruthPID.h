// Original licence header: SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten, Wouter Deconinck, Dmitry Romanov


#ifndef EICRECON_PARTICLESWITHTRUTHPID_H
#define EICRECON_PARTICLESWITHTRUTHPID_H

#include <algorithm>
#include <cmath>

#include <fmt/format.h>

#include <spdlog/spdlog.h>

// Event Model related classes
#include "edm4hep/MCParticleCollection.h"
#include "edm4eic/MCRecoParticleAssociationCollection.h"
#include "edm4eic/ReconstructedParticleCollection.h"
#include "edm4eic/TrackParametersCollection.h"
#include "edm4eic/vector_utils.h"

#include "ParticlesWithTruthPIDConfig.h"
#include "algorithms/reco/ParticlesWithAssociation.h"
#include <algorithms/interfaces/WithPodConfig.h>


namespace eicrecon {
    class ParticlesWithTruthPID : public WithPodConfig<ParticlesWithTruthPIDConfig> {

    public:

        void init(std::shared_ptr<spdlog::logger> logger) {
            m_log = logger;
        }

        ParticlesWithAssociation *execute(
                const std::vector<const edm4hep::MCParticle *> &mc_particles,
                const std::vector<const edm4eic::TrackParameters*> &track_params);

    private:

        std::shared_ptr<spdlog::logger> m_log;
    };
}

#endif //EICRECON_PARTICLESWITHTRUTHPID_H
