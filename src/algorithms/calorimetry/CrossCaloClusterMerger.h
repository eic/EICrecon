// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Tristan Protzman

/*
 * CrossCaloClusterMerger merges protoclusters across calorimeter
 * layers, weighting calorimeters appropriately.
 * 
 * Addresses issue #1561
*/

#pragma once

#include <algorithms/algorithm.h>
#include <algorithms/calorimetry/CrossCaloClusterMergerConfig.h>
#include <algorithms/interfaces/WithPodConfig.h>

#include <edm4eic/Cluster.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/ProtoCluster.h>
#include <edm4eic/ProtoClusterCollection.h>

#include <edm4hep/Vector3f.h>

#include <spdlog/spdlog.h>


namespace eicrecon {


    class CrossCaloClusterMerger : public WithPodConfig<CrossCaloClusterMergerConfig> {
    public:
        void init(std::shared_ptr<spdlog::logger>& logger);
        std::unique_ptr<edm4eic::ClusterCollection> execute (
            const edm4eic::ClusterCollection *collection_a,
            const edm4eic::ClusterCollection *collection_b
        );

    private:
        // Members
        std::shared_ptr<spdlog::logger> m_log;

        // Methods
        float get_cluster_energy(const edm4eic::ProtoCluster &clust);
        edm4hep::Vector3f get_cluster_position(const edm4eic::ProtoCluster &clust);
        void vacuum(std::unique_ptr<edm4eic::ClusterCollection> &output,
                    const edm4eic::ClusterCollection *collection_a,
                    const edm4eic::ClusterCollection *collection_b);

        void merge_by_eta_phi(std::unique_ptr<edm4eic::ClusterCollection> &output,
                    const edm4eic::ClusterCollection *collection_a,
                    const edm4eic::ClusterCollection *collection_b);

    };
} // namespace eicrecon