// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Tristan Protzman

/*
 * ProtoClusterMerger merges protoclusters across calorimeter
 * layers, weighting calorimeters appropriately.
 * 
 * Addresses issue #1561
*/

#pragma once

#include <algorithms/algorithm.h>
#include <algorithms/calorimetry/ProtoClusterMergerConfig.h>
#include <algorithms/interfaces/WithPodConfig.h>

#include <edm4eic/ProtoClusterCollection.h>

#include <spdlog/spdlog.h>


namespace eicrecon {

    class ProtoClusterMerger : public WithPodConfig<ProtoClusterMergerConfig> {
    public:
        void init(std::shared_ptr<spdlog::logger>& logger);
        std::unique_ptr<edm4eic::ProtoClusterCollection> execute (
            const edm4eic::ProtoClusterCollection *collection_a,
            const edm4eic::ProtoClusterCollection *collection_b
        );

    private:
        std::shared_ptr<spdlog::logger> m_log;

    };
} // namespace eicrecon