// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#include <memory>

#include <JANA/JEvent.h>

#include <spdlog/spdlog.h>

#include "InclusiveKinematicsSigma_factory.h"

#include <edm4hep/MCParticle.h>
#include <edm4eic/ReconstructedParticle.h>
#include <edm4eic/InclusiveKinematics.h>
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include <algorithms/tracking/ParticlesFromTrackFitResult.h>
#include "algorithms/reco/ParticlesWithAssociation.h"

namespace eicrecon {

    void InclusiveKinematicsSigma_factory::Init() {

        // This prefix will be used for parameters
        std::string param_prefix = "reco:" + GetTag();

        // Set input data tags properly
        InitDataTags(param_prefix);

        // SpdlogMixin logger initialization, sets m_log
        InitLogger(param_prefix, "info");

        m_inclusive_kinematics_algo.init(m_log);
    }

    void InclusiveKinematicsSigma_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
        // Nothing to do here
    }

    void InclusiveKinematicsSigma_factory::Process(const std::shared_ptr<const JEvent> &event) {
        auto mc_particles = event->Get<edm4hep::MCParticle>("MCParticles");
        auto rc_particles = event->Get<edm4eic::ReconstructedParticle>("ReconstructedChargedParticles");
        auto rc_particles_assoc = event->Get<edm4eic::MCRecoParticleAssociation>("ReconstructedChargedParticleAssociations");

        auto inclusive_kinematics = m_inclusive_kinematics_algo.execute(
            mc_particles,
            rc_particles,
            rc_particles_assoc
        );

        Set(inclusive_kinematics);
    }
} // eicrecon
