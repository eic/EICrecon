// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Sebouh Paul

#pragma once

#include <edm4eic/ClusterCollection.h>
#include <edm4hep/Vector4f.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <spdlog/logger.h>
#include <memory>
#include <string>                                 // for basic_string
#include <string_view>                            // for string_view
#include <algorithms/algorithm.h>
#include "algorithms/interfaces/WithPodConfig.h"


namespace eicrecon {

using CalorimeterTotalFourMomentumAlgorithm = algorithms::Algorithm<
   algorithms::Input<
       const edm4eic::ClusterCollection
    >,
    algorithms::Output<
       edm4eic::ReconstructedParticleCollection
    >
    >;
    class CalorimeterTotalFourMomentum :
       public CalorimeterTotalFourMomentumAlgorithm,
       public WithPodConfig<NoConfig> {
       public:
         CalorimeterTotalFourMomentum(std::string_view name)
                  : CalorimeterTotalFourMomentumAlgorithm{name,
                                        {"inputClusters"},
                                        {"totalFourMomentum"},
                                        "Merges all clusters in a collection into a total 4-vector of momentum and energy"} {}

         void init() final;
         void process(const Input&, const Output&) const final;
    private:
        std::shared_ptr<spdlog::logger> m_log;
    };
} // namespace eicrecon
