// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Tristan Protzman

#include <algorithms/calorimetry/ProtoClusterMerger.h>
#include <algorithms/calorimetry/ProtoClusterMergerConfig.h>

#include <edm4eic/ProtoClusterCollection.h>

#include <spdlog/spdlog.h>



namespace eicrecon {
    void ProtoClusterMerger::init(std::shared_ptr<spdlog::logger> &logger) {
        m_log = logger;
    }

    std::unique_ptr<edm4eic::ProtoClusterCollection> execute (
            const edm4eic::ProtoClusterCollection *collection_a,
            const edm4eic::ProtoClusterCollection *collection_b
        ) {
        
        // Define the output collection
        auto merged_clusters = std::make_unique<edm4eic::ProtoClusterCollection>();
        return merged_clusters;
    }
}

