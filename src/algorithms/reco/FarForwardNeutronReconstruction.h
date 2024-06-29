// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Sebouh Paul

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <spdlog/logger.h>
#include <memory>
#include <optional>
#include <string>                                 // for basic_string
#include <string_view>                            // for string_view
#include <vector>

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/reco/FarForwardNeutronReconstructionConfig.h"

namespace eicrecon {

using FarForwardNeutronReconstructionAlgorithm = algorithms::Algorithm<
   algorithms::Input<
       const edm4eic::ClusterCollection,
       std::optional<edm4eic::ClusterCollection>
    >,
    algorithms::Output<
       edm4eic::ReconstructedParticleCollection
    >
    >;
    class FarForwardNeutronReconstruction :
       public FarForwardNeutronReconstructionAlgorithm,
       public WithPodConfig<FarForwardNeutronReconstructionConfig> {
       public:
         FarForwardNeutronReconstruction(std::string_view name)
                  : FarForwardNeutronReconstructionAlgorithm{name,
                                        {"inputClustersHcal", "inputClustersEcal"},
                                        {"outputNeutrons"},
                                        "Merges all HCAL (and optionally also ECAL) clusters in a collection into a neutron candidate"} {}

         void init() final;
         void process(const Input&, const Output&) const final;
         double calc_corr(double Etot, const std::vector<double>&) const;
    private:
        std::shared_ptr<spdlog::logger> m_log;
        double m_neutron{0.93956542052};

    };
} // namespace eicrecon
