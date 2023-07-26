// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten

#include <limits>
//#include <numbers>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

// Event Model related classes
#include <edm4hep/MCParticle.h>
#include <edm4eic/Cluster.h>
#include <edm4eic/MCRecoClusterParticleAssociation.h>
#include <edm4eic/vector_utils.h>

//using namespace Gaudi::Units;

//namespace Jug::Fast {

/** Simple algorithm to merge the energy measurement from cluster1 with the position
 * measurement of cluster2 (in case matching clusters are found). If not, it will
 * propagate the raw cluster from cluster1 or cluster2
 *
 * Matching occurs based on the mc truth information of the clusters.
 *
 * \ingroup reco
 */
class TruthEnergyPositionClusterMerger {
protected:
    // Input
    std::vector<const edm4hep::MCParticle *> m_inputMCParticles;
    std::vector<const edm4eic::Cluster *> m_energyClusters;
    std::vector<const edm4eic::MCRecoClusterParticleAssociation *> m_energyAssociations;
    std::vector<const edm4eic::Cluster *> m_positionClusters;
    std::vector<const edm4eic::MCRecoClusterParticleAssociation *> m_positionAssociations;
    // Output
    std::vector<edm4eic::Cluster *> m_outputClusters;
    std::vector<edm4eic::MCRecoClusterParticleAssociation *> m_outputAssociations;

    std::shared_ptr<spdlog::logger> m_log;
public:
    TruthEnergyPositionClusterMerger() = default;

    void initialize() {}

    void execute() {
        if (m_log->level() == SPDLOG_LEVEL_DEBUG) {
            m_log->debug( "Merging energy and position clusters for new event" );
        }
        // input
        auto &mcparticles = m_inputMCParticles;
        auto &energy_clus = m_energyClusters;
        auto &energy_assoc = m_energyAssociations;
        auto &pos_clus = m_positionClusters;
        auto &pos_assoc = m_positionAssociations;
        // output
        auto &merged_clus = m_outputClusters;
        auto &merged_assoc = m_outputAssociations;

        if (!energy_clus.size() && !pos_clus.size()) {
            if (m_log->level() == SPDLOG_LEVEL_DEBUG) {
                m_log->debug( "Nothing to do for this event, returning..." );
            }
            return;
        }

        if (m_log->level() == SPDLOG_LEVEL_DEBUG) {
            m_log->debug( "Step 0/2: Getting indexed list of clusters..." );
        }
        // get an indexed map of all clusters
        auto energyMap = indexedClusters(energy_clus, energy_assoc);
        auto posMap = indexedClusters(pos_clus, pos_assoc);

        // loop over all position clusters and match with energy clusters
        if (m_log->level() == SPDLOG_LEVEL_DEBUG) {
            m_log->debug( "Step 1/2: Matching all position clusters to the available energy clusters..." );
        }
        for (const auto &[mcID, pclus]: posMap) {
            if (m_log->level() == SPDLOG_LEVEL_DEBUG) {
                m_log->debug( fmt::format(" --> Processing position cluster {}, mcID: {}, energy: {}", pclus.id(), mcID, pclus.getEnergy()));
            }
            if (energyMap.count(mcID)) {
                const auto &eclus = energyMap[mcID];
                edm4eic::MutableCluster new_clus;
                new_clus.setEnergy(eclus.getEnergy());
                new_clus.setEnergyError(eclus.getEnergyError());
                new_clus.setTime(pclus.getTime());
                new_clus.setNhits(pclus.getNhits() + eclus.getNhits());
                new_clus.setPosition(pclus.getPosition());
                new_clus.setPositionError(pclus.getPositionError());
//                new_clus.addToClusters(pclus);
//                new_clus.addToClusters(eclus);
                for (const auto &cl: {pclus, eclus}) {
                    for (const auto &hit: cl.getHits()) {
//                        new_clus.addToHits(hit);
                    }
                    new_clus.addToSubdetectorEnergies(cl.getEnergy());
                }
                for (const auto &param: pclus.getShapeParameters()) {
                    new_clus.addToShapeParameters(param);
                }
                if (m_log->level() == SPDLOG_LEVEL_DEBUG) {
                    m_log->debug( fmt::format("   --> Found matching energy cluster {}, energy: {}", eclus.id(), eclus.getEnergy() ));
                    m_log->debug( fmt::format("   --> Created new combined cluster {}, energy: ", new_clus.id(), new_clus.getEnergy() ));
                }
                auto cl = new edm4eic::Cluster( new_clus );
                merged_clus.push_back( cl );

                // set association
                edm4eic::MutableMCRecoClusterParticleAssociation clusterassoc;
                clusterassoc.setRecID(cl->getObjectID().index);
                clusterassoc.setSimID(mcID);
                clusterassoc.setWeight(1.0);
                clusterassoc.setRec(*cl);
                //clusterassoc.setSim(mcparticles[mcID]);
                merged_assoc.push_back(new edm4eic::MCRecoClusterParticleAssociation(clusterassoc));

                // erase the energy cluster from the map, so we can in the end account for all
                // remaining clusters
                energyMap.erase(mcID);
            } else {
                if (m_log->level() == SPDLOG_LEVEL_DEBUG) {
                    m_log->debug("   --> No matching energy cluster found, copying over position cluster" );
                }
                auto new_clus = pclus.clone();
//                new_clus.addToClusters(pclus);
                auto cl = new edm4eic::Cluster(new_clus);
                merged_clus.push_back(cl);

                // set association
                edm4eic::MutableMCRecoClusterParticleAssociation clusterassoc;
                clusterassoc.setRecID(cl->getObjectID().index);
                clusterassoc.setSimID(mcID);
                clusterassoc.setWeight(1.0);
                clusterassoc.setRec(*cl);
                //clusterassoc.setSim(mcparticles[mcID]);
                merged_assoc.push_back(new edm4eic::MCRecoClusterParticleAssociation(clusterassoc));
            }
        }
        // Collect remaining energy clusters. Use mc truth position for these clusters, as
        // they should really have a match in the position clusters (and if they don't it due
        // to a clustering error).
        if (m_log->level() == SPDLOG_LEVEL_DEBUG) {
            m_log->debug( "Step 2/2: Collecting remaining energy clusters..." );
        }
        for (const auto &[mcID, eclus]: energyMap) {
            const auto &mc = mcparticles[mcID];
            const auto &p = mc->getMomentum();
            const auto phi = std::atan2(p.y, p.x);
            const auto theta = std::atan2(std::hypot(p.x, p.y), p.z);
            edm4eic::MutableCluster  new_clus;
            new_clus.setEnergy(eclus.getEnergy());
            new_clus.setEnergyError(eclus.getEnergyError());
            new_clus.setTime(eclus.getTime());
            new_clus.setNhits(eclus.getNhits());
            // use nominal dd4hep::radius of 110cm, and use start vertex theta and phi
            new_clus.setPosition(edm4eic::sphericalToVector(110. * dd4hep::cm, theta, phi));
//            new_clus.addToClusters(eclus);
            if (m_log->level() == SPDLOG_LEVEL_DEBUG) {
                m_log->debug( fmt::format(" --> Processing energy cluster {}, mcID: {}, energy: {}", eclus.id() ,mcID ,eclus.getEnergy() ));
                m_log->debug( fmt::format("   --> Created new 'combined' cluster {}, energy: {}", new_clus.id(),new_clus.getEnergy() ));
            }
            auto cl = new edm4eic::Cluster( new_clus );
            merged_clus.push_back( cl );

            // set association
            edm4eic::MutableMCRecoClusterParticleAssociation clusterassoc;
            clusterassoc.setRecID(cl->getObjectID().index);
            clusterassoc.setSimID(mcID);
            clusterassoc.setWeight(1.0);
            clusterassoc.setRec(*cl);
            //clusterassoc.setSim(mc);
            merged_assoc.push_back(new edm4eic::MCRecoClusterParticleAssociation(clusterassoc));
        }
    }

    // get a map of MCParticle index --> cluster
    // input: cluster_collections --> list of handles to all cluster collections
    std::map<int, edm4eic::Cluster> indexedClusters(
            std::vector<const edm4eic::Cluster*> &clusters,
            std::vector<const edm4eic::MCRecoClusterParticleAssociation*> &associations
    ) const {

        std::map<int, edm4eic::Cluster> matched = {};

        for (const auto &cluster: clusters) {
            int mcID = -1;

            // find associated particle
            for (const auto &assoc: associations) {
                if (assoc->getRec() == *cluster) {
                    mcID = assoc->getSimID();
                    break;
                }
            }

            if (m_log->level() == SPDLOG_LEVEL_TRACE) {
                m_log->trace( fmt::format( " --> Found cluster: {} with mcID {} and energy {}", cluster->getObjectID().index, mcID, cluster->getEnergy() ));
            }

            if (mcID < 0) {
                if (m_log->level() == SPDLOG_LEVEL_TRACE) {
                    m_log->trace( "   --> WARNING: no valid MC truth link found, skipping cluster..." );
                }
                continue;
            }

            const bool duplicate = matched.count(mcID);
            if (duplicate) {
                if (m_log->level() == SPDLOG_LEVEL_TRACE) {
                    m_log->trace( "   --> WARNING: this is a duplicate mcID, keeping the higher energy cluster" );
                }
                if (cluster->getEnergy() < matched[mcID].getEnergy()) {
                    continue;
                }
            }

            matched[mcID] = *cluster;
        }
        return matched;
    }
};

//} // namespace Jug::Fast
