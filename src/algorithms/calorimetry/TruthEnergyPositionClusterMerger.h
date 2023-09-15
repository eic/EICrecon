// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten

#pragma once

#include <limits>

#include <spdlog/spdlog.h>

// Event Model related classes
#include <edm4hep/MCParticle.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/Cluster.h>
#include <edm4eic/MCRecoClusterParticleAssociation.h>
#include <edm4eic/vector_utils.h>

#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

/** Simple algorithm to merge the energy measurement from cluster1 with the position
 * measurement of cluster2 (in case matching clusters are found). If not, it will
 * propagate the raw cluster from cluster1 or cluster2
 *
 * Matching occurs based on the mc truth information of the clusters.
 *
 * \ingroup reco
 */
  class TruthEnergyPositionClusterMerger : WithPodConfig<NoConfig> {

  using ClustersWithAssociations = std::tuple<
        std::unique_ptr<edm4eic::ClusterCollection>,
        std::unique_ptr<edm4eic::MCRecoClusterParticleAssociationCollection>
  >;

  protected:

    std::shared_ptr<spdlog::logger> m_log;

  public:

    void init(std::shared_ptr<spdlog::logger>& logger) {
        m_log = logger;
    }

    ClustersWithAssociations process(
        const edm4hep::MCParticleCollection& mcparticles,
        const edm4eic::ClusterCollection& energy_clus,
        const edm4eic::MCRecoClusterParticleAssociationCollection& energy_assoc,
        const edm4eic::ClusterCollection& pos_clus,
        const edm4eic::MCRecoClusterParticleAssociationCollection& pos_assoc
    ) {

        m_log->debug( "Merging energy and position clusters for new event" );

        // output
        auto merged_clus = std::make_unique<edm4eic::ClusterCollection>();
        auto merged_assoc = std::make_unique<edm4eic::MCRecoClusterParticleAssociationCollection>();

        if (energy_clus.size() == 0 && pos_clus.size() == 0) {
            m_log->debug( "Nothing to do for this event, returning..." );
            return std::make_tuple(std::move(merged_clus), std::move(merged_assoc));
        }

        m_log->debug( "Step 0/2: Getting indexed list of clusters..." );

        // get an indexed map of all clusters
        m_log->debug(" --> Indexing energy clusters");
        auto energyMap = indexedClusters(energy_clus, energy_assoc);
        m_log->trace(" --> Found these energy clusters:");
        for (const auto &[mcID, eclus]: energyMap) {
            m_log->trace("   --> energy cluster {}, mcID: {}, energy: {}", eclus.getObjectID().index, mcID, eclus.getEnergy());
        }
        m_log->debug(" --> Indexing position clusters");
        auto posMap = indexedClusters(pos_clus, pos_assoc);
        m_log->trace(" --> Found these position clusters:");
        for (const auto &[mcID, pclus]: posMap) {
            m_log->trace("   --> position cluster {}, mcID: {}, energy: {}", pclus.getObjectID().index, mcID, pclus.getEnergy());
        }

        // loop over all position clusters and match with energy clusters
        m_log->debug( "Step 1/2: Matching all position clusters to the available energy clusters..." );
        for (const auto &[mcID, pclus]: posMap) {

            m_log->debug(" --> Processing position cluster {}, mcID: {}, energy: {}", pclus.getObjectID().index, mcID, pclus.getEnergy());
            if (energyMap.count(mcID)) {

                const auto &eclus = energyMap[mcID];

                auto new_clus = merged_clus->create();
                new_clus.setEnergy(eclus.getEnergy());
                new_clus.setEnergyError(eclus.getEnergyError());
                new_clus.setTime(pclus.getTime());
                new_clus.setNhits(pclus.getNhits() + eclus.getNhits());
                new_clus.setPosition(pclus.getPosition());
                new_clus.setPositionError(pclus.getPositionError());
                new_clus.addToClusters(pclus);
                new_clus.addToClusters(eclus);
                for (const auto &cl: {pclus, eclus}) {
                    for (const auto &hit: cl.getHits()) {
                        new_clus.addToHits(hit);
                    }
                    new_clus.addToSubdetectorEnergies(cl.getEnergy());
                }
                for (const auto &param: pclus.getShapeParameters()) {
                    new_clus.addToShapeParameters(param);
                }
                m_log->debug("   --> Found matching energy cluster {}, energy: {}", eclus.getObjectID().index, eclus.getEnergy() );
                m_log->debug("   --> Created new combined cluster {}, energy: {}", new_clus.getObjectID().index, new_clus.getEnergy() );

                // set association
                auto clusterassoc = merged_assoc->create();
#if EDM4EIC_VERSION_MAJOR < 4
                clusterassoc.setRecID(new_clus.getObjectID().index);
                clusterassoc.setSimID(mcID);
#endif
                clusterassoc.setWeight(1.0);
                clusterassoc.setRec(new_clus);
                clusterassoc.setSim(mcparticles[mcID]);

                // erase the energy cluster from the map, so we can in the end account for all
                // remaining clusters
                energyMap.erase(mcID);
            } else {
                m_log->debug("   --> No matching energy cluster found, copying over position cluster" );
                auto new_clus = pclus.clone();
                new_clus.addToClusters(pclus);
                merged_clus->push_back(new_clus);

                // set association
                auto clusterassoc = merged_assoc->create();
#if EDM4EIC_VERSION_MAJOR < 4
                clusterassoc.setRecID(new_clus.getObjectID().index);
                clusterassoc.setSimID(mcID);
#endif
                clusterassoc.setWeight(1.0);
                clusterassoc.setRec(new_clus);
                clusterassoc.setSim(mcparticles[mcID]);
            }
        }
        // Collect remaining energy clusters. Use mc truth position for these clusters, as
        // they should really have a match in the position clusters (and if they don't it due
        // to a clustering error).
        m_log->debug( "Step 2/2: Collecting remaining energy clusters..." );
        for (const auto &[mcID, eclus]: energyMap) {
            const auto &mc = mcparticles[mcID];
            const auto &p = mc.getMomentum();
            const auto phi = std::atan2(p.y, p.x);
            const auto theta = std::atan2(std::hypot(p.x, p.y), p.z);

            auto new_clus = merged_clus->create();
            new_clus.setEnergy(eclus.getEnergy());
            new_clus.setEnergyError(eclus.getEnergyError());
            new_clus.setTime(eclus.getTime());
            new_clus.setNhits(eclus.getNhits());
            // FIXME use nominal dd4hep::radius of 110cm, and use start vertex theta and phi
            new_clus.setPosition(edm4eic::sphericalToVector(78.5 * dd4hep::cm / dd4hep::mm, theta, phi));
            new_clus.addToClusters(eclus);

            m_log->debug(" --> Processing energy cluster {}, mcID: {}, energy: {}", eclus.getObjectID().index, mcID, eclus.getEnergy() );
            m_log->debug("   --> Created new 'combined' cluster {}, energy: {}", new_clus.getObjectID().index, new_clus.getEnergy() );

            // set association
            auto clusterassoc = merged_assoc->create();
#if EDM4EIC_VERSION_MAJOR < 4
            clusterassoc.setRecID(new_clus.getObjectID().index);
            clusterassoc.setSimID(mcID);
#endif
            clusterassoc.setWeight(1.0);
            clusterassoc.setRec(new_clus);
            clusterassoc.setSim(mc);
        }

        return std::make_tuple(std::move(merged_clus), std::move(merged_assoc));
    }

    // get a map of MCParticle index --> cluster
    // input: cluster_collections --> list of handles to all cluster collections
    std::map<int, edm4eic::Cluster> indexedClusters(
            const edm4eic::ClusterCollection& clusters,
            const edm4eic::MCRecoClusterParticleAssociationCollection& associations
    ) const {

        std::map<int, edm4eic::Cluster> matched = {};

        for (const auto &cluster: clusters) {
            int mcID = -1;

            // find associated particle
            for (const auto &assoc: associations) {
                if (assoc.getRec() == cluster) {
                    mcID = assoc.getSim().getObjectID().index;
                    break;
                }
            }

            m_log->trace(" --> Found cluster: {} with mcID {} and energy {}", cluster.getObjectID().index, mcID, cluster.getEnergy());

            if (mcID < 0) {
                m_log->trace( "   --> WARNING: no valid MC truth link found, skipping cluster..." );
                continue;
            }

            const bool duplicate = matched.count(mcID);
            if (duplicate) {
                m_log->trace( "   --> WARNING: this is a duplicate mcID, keeping the higher energy cluster");
                if (cluster.getEnergy() < matched[mcID].getEnergy()) {
                    continue;
                }
            }

            matched[mcID] = cluster;
        }
        return matched;
    }
};

} // namespace eicrecon
