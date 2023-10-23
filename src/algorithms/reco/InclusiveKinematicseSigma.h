// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023 Sylvester Joosten, Dmitry Romanov, Wouter Deconinck

#pragma once

#include <edm4eic/InclusiveKinematicsCollection.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
// Event Model related classes
#include <edm4hep/MCParticleCollection.h>
#include <spdlog/logger.h>
#include <memory>


namespace eicrecon {

    class InclusiveKinematicseSigma {

    public:

        void init(std::shared_ptr<spdlog::logger> logger);

        std::unique_ptr<edm4eic::InclusiveKinematicsCollection> execute(
                const edm4hep::MCParticleCollection& mcparts,
                const edm4eic::ReconstructedParticleCollection& rcparts,
                const edm4eic::MCRecoParticleAssociationCollection& rcassoc
        );

    private:
        std::shared_ptr<spdlog::logger> m_log;
        double m_proton{0.93827}, m_neutron{0.93957}, m_electron{0.000510998928}, m_crossingAngle{-0.025};
    };

} // namespace eicrecon
