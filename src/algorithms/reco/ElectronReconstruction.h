// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Daniel Brandenburg

#pragma once

#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <spdlog/logger.h>
#include <memory>
#include <vector>

#include "ElectronReconstructionConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"


namespace eicrecon {

    class ElectronReconstruction : public WithPodConfig<ElectronReconstructionConfig>{

    public:

        void init(std::shared_ptr<spdlog::logger> logger);

        // idea will be to overload this with other version (e.g. reco mode)
        std::unique_ptr<edm4eic::ReconstructedParticleCollection> execute(
                const edm4hep::MCParticleCollection *mcparts,
                const edm4eic::ReconstructedParticleCollection *rcparts,
                const edm4eic::MCRecoParticleAssociationCollection *rcassoc,
                const std::vector<const edm4eic::MCRecoClusterParticleAssociationCollection*> &in_clu_assoc
        );

    private:
        std::shared_ptr<spdlog::logger> m_log;
        double m_electron{0.000510998928};

    };
} // namespace eicrecon
