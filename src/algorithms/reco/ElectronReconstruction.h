// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Daniel Brandenburg

#pragma once

#include <algorithm>
#include <cmath>
#include <vector>
#include <map>

#include <spdlog/spdlog.h>

#include <edm4eic/vector_utils.h>

// Event Model related classes
#include "edm4hep/MCParticleCollection.h"
#include "edm4eic/MCRecoParticleAssociationCollection.h"
#include "edm4eic/ReconstructedParticleCollection.h"
#include "edm4eic/MCRecoClusterParticleAssociation.h"


namespace eicrecon {

    class ElectronReconstruction {

    public:

        void init(std::shared_ptr<spdlog::logger> logger);

        // idea will be to overload this with other version (e.g. reco mode)
        std::vector<edm4eic::ReconstructedParticle*> execute(
                std::vector<const edm4hep::MCParticle*> mcparts,
                std::vector<const edm4eic::ReconstructedParticle*> rcparts,
                std::vector<const edm4eic::MCRecoParticleAssociation*> rcassoc,
                std::vector<std::vector<const edm4eic::MCRecoClusterParticleAssociation*>> &in_clu_assoc
        );

        void setEnergyOverMomentumCut( double minEoP, double maxEoP ) { min_energy_over_momentum = minEoP; max_energy_over_momentum = maxEoP; }

    private:
        std::shared_ptr<spdlog::logger> m_log;
        double m_proton{0.93827}, m_neutron{0.93957}, m_electron{0.000510998928}, m_crossingAngle{-0.025};
        double min_energy_over_momentum{0.9}, max_energy_over_momentum{1.2};

    };
} // namespace eicrecon
