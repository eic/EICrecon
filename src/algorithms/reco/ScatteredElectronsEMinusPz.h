// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Daniel Brandenburg

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <spdlog/logger.h>
#include <memory>
#include <vector>

#include "ScatteredElectronsEMinusPzConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"


namespace eicrecon {

    class ScatteredElectronsEMinusPz : public WithPodConfig<ScatteredElectronsEMinusPzConfig>{

    public:

        void init(std::shared_ptr<spdlog::logger>& logger);
        std::unique_ptr<edm4eic::ReconstructedParticleCollection> execute(
                const edm4eic::ReconstructedParticleCollection *rcparts,
                const edm4eic::ReconstructedParticleCollection *rcele
        );

    private:
        std::shared_ptr<spdlog::logger> m_log;
        double m_electron{0.000510998928}, m_pion{0.13957};

    };
} // namespace eicrecon
