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

#include <edm4eic/ProtoCluster.h>
#include <edm4eic/ProtoClusterCollection.h>

#include <edm4hep/Vector3f.h>

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
        // Members
        std::shared_ptr<spdlog::logger> m_log;

        // Methods
        float get_cluster_energy(const edm4eic::ProtoCluster &clust);
        edm4hep::Vector3f get_cluster_position(const edm4eic::ProtoCluster &clust);
        void vacuum(std::unique_ptr<edm4eic::ProtoClusterCollection> &output,
                    const edm4eic::ProtoClusterCollection *collection_a,
                    const edm4eic::ProtoClusterCollection *collection_b);

        void merge_by_eta_phi(std::unique_ptr<edm4eic::ProtoClusterCollection> &output,
                    const edm4eic::ProtoClusterCollection *collection_a,
                    const edm4eic::ProtoClusterCollection *collection_b);

    };
} // namespace eicrecon