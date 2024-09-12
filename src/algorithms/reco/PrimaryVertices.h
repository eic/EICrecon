// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Xin Dong

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/VertexCollection.h>
#include <spdlog/logger.h>
#include <memory>
#include <optional>
#include <string>                                 // for basic_string
#include <string_view>                            // for string_view

#include "algorithms/reco/PrimaryVerticesConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"


namespace eicrecon {

using PrimaryVerticesAlgorithm = algorithms::Algorithm<
   algorithms::Input<edm4eic::VertexCollection>,
   algorithms::Output<edm4eic::VertexCollection>>;

    class PrimaryVertices :
       public PrimaryVerticesAlgorithm,
       public WithPodConfig<PrimaryVerticesConfig>{

       public:
         PrimaryVertices(std::string_view name)
              : PrimaryVerticesAlgorithm{name,
                             {"inputVertices"},
                             {"outputPrimaryVertices"},
                             "Sort reconstructed vertices in PrimaryVertices collection"} {}

         void init() final;
         void process(const Input&, const Output&) const final;

    private:

    };
} // namespace eicrecon
