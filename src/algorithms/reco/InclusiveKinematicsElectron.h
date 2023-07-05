// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023 Sylvester Joosten, Dmitry Romanov, Wouter Deconinck

#pragma once

#include <algorithm>
#include <cmath>
#include <vector>
#include <map>

#include <spdlog/spdlog.h>


// Event Model related classes
#include "edm4hep/MCParticleCollection.h"
#include "edm4eic/MCRecoParticleAssociationCollection.h"
#include "edm4eic/ReconstructedParticleCollection.h"
#include "edm4eic/InclusiveKinematicsCollection.h"


namespace eicrecon {

    class InclusiveKinematicsElectron {

    public:

        void init(std::shared_ptr<spdlog::logger> logger);

        std::vector<edm4eic::InclusiveKinematics*> execute(
                std::vector<const edm4hep::MCParticle*> mcparts,
                std::vector<const edm4eic::ReconstructedParticle*> rcparts,
                std::vector<const edm4eic::MCRecoParticleAssociation*> rcassoc
        );

    private:
        std::shared_ptr<spdlog::logger> m_log;
        double m_proton{0.93827}, m_neutron{0.93957}, m_electron{0.000510998928}, m_crossingAngle{-0.025};
        };

} // namespace eicrecon
