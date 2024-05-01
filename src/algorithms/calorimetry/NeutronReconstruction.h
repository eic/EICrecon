// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Sebouh Paul

#pragma once

#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/ClusterCollection.h>
#include <spdlog/logger.h>
#include <memory>
#include <gsl/pointers>
#include <string>                                 // for basic_string
#include <string_view>                            // for string_view

#include <stdlib.h>
#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include "NeutronReconstructionConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"


namespace eicrecon {

using NeutronReconstructionAlgorithm = algorithms::Algorithm<
   algorithms::Input<
       const edm4eic::ClusterCollection
    >,
    algorithms::Output<
       edm4eic::ReconstructedParticleCollection
    >
    >;
    class NeutronReconstruction :
       public NeutronReconstructionAlgorithm,
       public WithPodConfig<NeutronReconstructionConfig> {
       public:
         NeutronReconstruction(std::string_view name)
                  : NeutronReconstructionAlgorithm{name,
                                        {"inputClusters"},
                                        {"outputNeutrons"},
                                        "Merges all HCAL clusters in a collection into a neutron candidate"} {}

         void init() final;
         void process(const Input&, const Output&) const final;
    private:
        std::shared_ptr<spdlog::logger> m_log;
        double m_neutron{0.93956542052};

    };
} // namespace eicrecon
