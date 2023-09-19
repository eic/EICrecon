// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Daniel Brandenburg

#pragma once

#include <memory>
#include <vector>

namespace edm4eic { class MCRecoClusterParticleAssociationCollection; }
namespace edm4eic { class MCRecoParticleAssociationCollection; }
namespace edm4eic { class ReconstructedParticleCollection; }
namespace edm4hep { class MCParticleCollection; }
namespace spdlog { class logger; }


namespace eicrecon {

    class ElectronReconstruction {

    public:

        void init(std::shared_ptr<spdlog::logger> logger);

        // idea will be to overload this with other version (e.g. reco mode)
        std::unique_ptr<edm4eic::ReconstructedParticleCollection> execute(
                const edm4hep::MCParticleCollection *mcparts,
                const edm4eic::ReconstructedParticleCollection *rcparts,
                const edm4eic::MCRecoParticleAssociationCollection *rcassoc,
                const std::vector<const edm4eic::MCRecoClusterParticleAssociationCollection*> &in_clu_assoc
        );

        void setEnergyOverMomentumCut( double minEoP, double maxEoP ) { min_energy_over_momentum = minEoP; max_energy_over_momentum = maxEoP; }

    private:
        std::shared_ptr<spdlog::logger> m_log;
        double m_electron{0.000510998928};
        double min_energy_over_momentum{0.9}, max_energy_over_momentum{1.2};

    };
} // namespace eicrecon
