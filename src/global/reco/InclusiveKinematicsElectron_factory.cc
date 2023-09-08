// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#include <JANA/JEvent.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <memory>

#include "InclusiveKinematicsElectron_factory.h"
#include "datamodel_glue.h"

namespace eicrecon {

    void InclusiveKinematicsElectron_factory::Init() {

        // This prefix will be used for parameters
        std::string param_prefix = "reco:" + GetTag();

        // SpdlogMixin logger initialization, sets m_log
        InitLogger(GetApplication(), param_prefix, "info");

        m_inclusive_kinematics_algo.init(m_log);
    }

    void InclusiveKinematicsElectron_factory::Process(const std::shared_ptr<const JEvent> &event) {
        const auto* mc_particles = event->GetCollection<edm4hep::MCParticle>(GetInputTags()[0]);
        const auto* rc_particles = event->GetCollection<edm4eic::ReconstructedParticle>(GetInputTags()[1]);
        const auto* rc_particles_assoc = event->GetCollection<edm4eic::MCRecoParticleAssociation>(GetInputTags()[2]);

        auto inclusive_kinematics = m_inclusive_kinematics_algo.execute(
            *mc_particles,
            *rc_particles,
            *rc_particles_assoc
        );

        SetCollection<edm4eic::InclusiveKinematics>(GetOutputTags()[0], std::move(inclusive_kinematics));
    }
} // eicrecon
