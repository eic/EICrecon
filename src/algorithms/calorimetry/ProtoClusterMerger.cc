// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Tristan Protzman

#include <list>

#include <algorithms/calorimetry/ProtoClusterMerger.h>
#include <algorithms/calorimetry/ProtoClusterMergerConfig.h>

#include <edm4eic/CalorimeterHit.h>
#include <edm4eic/ProtoCluster.h>
#include <edm4eic/ProtoClusterCollection.h>

#include <edm4hep/utils/vector_utils.h>
#include <edm4hep/Vector3f.h>

#include <spdlog/spdlog.h>


namespace eicrecon {
    float ProtoClusterMerger::get_cluster_energy(const edm4eic::ProtoCluster &clust) {
        float energy = 0;
        for (auto hit : clust.getHits()) {
            energy += hit.getEnergy();
        }
        return energy;
    }

    // --------------------------------------------------------------------------
    // Get current center of protocluster
    // From Derek, this really should be centralized into some kind of edm4eic::utils
    // collection but this works temporarily
    // --------------------------------------------------------------------------
    edm4hep::Vector3f ProtoClusterMerger::get_cluster_position(const edm4eic::ProtoCluster &clust) {
        // grab total energy
        const float eClust = get_cluster_energy(clust);

        // calculate energy-weighted center
        float wTotal = 0.;
        edm4hep::Vector3f position(0., 0., 0.);
        for (auto hit : clust.getHits()) {
            // calculate weight
            float weight = hit.getEnergy() / eClust;
            wTotal += weight;

            // update cluster position
            position = position + (hit.getPosition() * weight);
        }

    float norm = 1.;
    if (wTotal == 0.) {
        // warning("Total weight of 0 in position calculation!");
    } else {
        norm = wTotal;
    }
    return position / norm;
    } 

    void ProtoClusterMerger::init(std::shared_ptr<spdlog::logger> &logger) {
        m_log = logger;
    }

    std::unique_ptr<edm4eic::ProtoClusterCollection> ProtoClusterMerger::execute (
            const edm4eic::ProtoClusterCollection *collection_a,
            const edm4eic::ProtoClusterCollection *collection_b
        ) {
        
        // Define the output collection
        auto merged_clusters = std::make_unique<edm4eic::ProtoClusterCollection>();
        
        if (m_cfg.merge_scheme == 0) {
            // No mergeing clusters, just output a collection with all clusters
            // from both calorimeters (vacuum mode)
            vacuum(merged_clusters, collection_a, collection_b);
        }
        else if (m_cfg.merge_scheme == 1) {
            // Merge by eta/phi less than dR
            merge_by_eta_phi(merged_clusters, collection_a, collection_b);
        }
        return merged_clusters;
    }

    // Create a collection including all the clusters from both collections, with no merging
    void ProtoClusterMerger::vacuum(std::unique_ptr<edm4eic::ProtoClusterCollection> &output,
                                    const edm4eic::ProtoClusterCollection *collection_a,
                                    const edm4eic::ProtoClusterCollection *collection_b) {
        // Since we aren't merging any clusters, the output collection can be made a subset 
        // collection (I think) containing no new items  TODO:  Test limits of subset collections
        output->setSubsetCollection();
        for (auto cluster : *collection_a) {
            output->push_back(cluster);
        }
        for (auto cluster : *collection_b) {
            output->push_back(cluster);
        }
    }

    // Create a collection of clusters merging clusters with dR < sqrt(dEta^2 + dPhi^2)
    // TODO:
    // * Clusters are merged with equal weights, need to adjust for sampling fraction/energy
    // * Don't yet check which calorimeter hits are from -> can't select proper sampling fraction
    // * Is dR the right metric?  What about in the forward region
    void ProtoClusterMerger::merge_by_eta_phi(std::unique_ptr<edm4eic::ProtoClusterCollection> &output,
                                              const edm4eic::ProtoClusterCollection *collection_a,
                                              const edm4eic::ProtoClusterCollection *collection_b) {
        // Create a list of indices for collection B to track which are matched
        std::list<unsigned int> indices;
        for (uint i = 0; i < collection_b->size(); i++) {
            indices.push_back(i);
        }
        // For each cluster A, find the nearest cluster B
        for (auto first : *collection_a) {
            float closest = (m_cfg.barrel_merge_distance + 1) * (m_cfg.barrel_merge_distance + 1);
            auto first_position = get_cluster_position(first);
            int closest_index = 0;
            for (auto i : indices) {
                auto second_position = get_cluster_position(collection_b->at(i));
                float deta = edm4hep::utils::eta(first_position) - edm4hep::utils::eta(second_position);
                float dphi = edm4hep::utils::angleBetween(edm4hep::utils::vectorTransverse(first_position), edm4hep::utils::vectorTransverse(second_position));
                float dr = (deta * deta) + (dphi * dphi);
                if (dr < closest) {
                    closest = dr;
                    closest_index = i;
                }
            }
            // If there is a cluster B within R of cluster A, merge them and add them to the output
            if (closest < m_cfg.barrel_merge_distance) {
                auto protocluster = output->create();
                for (auto hit : first.getHits()) {
                    protocluster.addToHits(hit);
                }
                for (auto hit : collection_b->at(closest_index).getHits()) {
                    protocluster.addToHits(hit);
                }
                output->push_back(protocluster);
                indices.erase(std::find(indices.begin(), indices.end(), closest_index));
            }
            else {  // Otherwise we add cluster A on its own to the final set of clusters
                output->push_back(first);
            }
        }
        // Add the remaining, unmatched cluster B to the final set of clusters
        for (auto i : indices) {
            output->push_back(collection_b->at(i));
        }
    } 
}
