// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#include <memory>

#include <JANA/JEvent.h>

#include <spdlog/spdlog.h>

#include "InclusiveKinematicsTruth_factory.h"

#include <edm4hep/MCParticleCollection.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include "extensions/spdlog/SpdlogExtensions.h"

namespace eicrecon {

    void InclusiveKinematicsTruth_factory::Init() {

        // This prefix will be used for parameters
        std::string param_prefix = "reco:" + GetTag();

        // SpdlogMixin logger initialization, sets m_log
        InitLogger(GetApplication(), param_prefix, "info");

        m_inclusive_kinematics_algo.init(m_log);
    }

    void InclusiveKinematicsTruth_factory::Process(const std::shared_ptr<const JEvent> &event) {
        const auto* mc_particles = static_cast<const edm4hep::MCParticleCollection*>(event->GetCollectionBase(GetInputTags()[0]));

        auto inclusive_kinematics = m_inclusive_kinematics_algo.execute(
            *mc_particles
        );

        SetCollection<edm4eic::InclusiveKinematics>(GetOutputTags()[0], std::move(inclusive_kinematics));
    }
} // eicrecon
