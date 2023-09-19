// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023 Sylvester Joosten, Dmitry Romanov, Wouter Deconinck

#pragma once






#include <memory>

namespace edm4eic { class InclusiveKinematicsCollection; }
namespace edm4eic { class MCRecoParticleAssociationCollection; }
namespace edm4eic { class ReconstructedParticleCollection; }
namespace edm4hep { class MCParticleCollection; }
namespace spdlog { class logger; }

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
