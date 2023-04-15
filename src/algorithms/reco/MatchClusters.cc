// Original header license: SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten, Dmitry Romanov

// Takes a list of particles (presumed to be from tracking), and all available clusters.
// 1. Match clusters to their tracks using the mcID field
// 2. For unmatched clusters create neutrals and add to the particle list

#include <algorithm>
#include <cmath>
#include <vector>
#include <map>

#include <spdlog/spdlog.h>
#include <fmt/format.h>

#include "MatchClusters.h"


// Event Model related classes
#include "edm4hep/MCParticleCollection.h"
#include "edm4eic/ClusterCollection.h"
#include "edm4eic/MCRecoClusterParticleAssociationCollection.h"
#include "edm4eic/MCRecoParticleAssociationCollection.h"
#include "edm4eic/ReconstructedParticleCollection.h"
#include "edm4eic/TrackParametersCollection.h"
#include "edm4eic/vector_utils.h"



namespace eicrecon {

    void MatchClusters::init(std::shared_ptr<spdlog::logger> logger) {
        m_log = logger;
    }

    std::tuple<edm4eic::ReconstructedParticleCollection*, edm4eic::MCRecoParticleAssociationCollection*> MatchClusters::execute(
            std::vector<const edm4hep::MCParticle *> mcparticles,
            std::vector<edm4eic::ReconstructedParticle *> inparts,
            std::vector<edm4eic::MCRecoParticleAssociation *> inpartsassoc,
            const std::vector<std::vector<const edm4eic::Cluster*>> &cluster_collections,
            const std::vector<std::vector<const edm4eic::MCRecoClusterParticleAssociation*>> &cluster_assoc_collections) {

        m_log->debug("Processing cluster info for new event");

        // Resulting reconstructed particles and associations
        edm4eic::ReconstructedParticleCollection* outparts = new edm4eic::ReconstructedParticleCollection();
        edm4eic::MCRecoParticleAssociationCollection* outpartsassoc = new edm4eic::MCRecoParticleAssociationCollection();

        m_log->debug("Step 0/2: Getting indexed list of clusters...");

        // get an indexed map of all clusters
        auto clusterMap = indexedClusters(cluster_collections, cluster_assoc_collections);

        // 1. Loop over all tracks and link matched clusters where applicable
        // (removing matched clusters from the cluster maps)
        m_log->debug("Step 1/2: Matching clusters to charged particles...");

        for (const auto &inpart: inparts) {
            m_log->debug(" --> Processing charged particle {}, PDG {}, energy {}", inpart->getObjectID().index,
                         inpart->getPDG(), inpart->getEnergy());

            auto outpart = outparts->create();

            int mcID = -1;

            // find associated particle
            for (const auto &assoc: inpartsassoc) {
                if (assoc->getRecID() == inpart->getObjectID().index) {
                    mcID = assoc->getSimID();
                    break;
                }
            }

            m_log->trace("    --> Found particle with mcID {}", mcID);

            if (mcID < 0) {
                m_log->debug("    --> cannot match track without associated mcID");
                continue;
            }

            if (clusterMap.count(mcID)) {
                const auto &clus = clusterMap[mcID];
                m_log->debug("    --> found matching cluster with energy: {}", clus->getEnergy());
                clusterMap.erase(mcID);
            }

            // create truth associations
            auto assoc = outpartsassoc->create();
            assoc.setRecID(outpart.getObjectID().index);
            assoc.setSimID(mcID);
            assoc.setWeight(1.0);
            assoc.setRec(outpart);
            assoc.setRec(outpart);
        }

        // 2. Now loop over all remaining clusters and add neutrals. Also add in Hcal energy
        // if a matching cluster is available
        m_log->debug("Step 2/2: Creating neutrals for remaining clusters...");
        for (const auto &[mcID, clus]: clusterMap) {
            m_log->debug(" --> Processing unmatched cluster with energy: {}", clus->getEnergy());


            // get mass/PDG from mcparticles, 0 (unidentified) in case the matched particle is charged.
            const auto &mc = mcparticles[mcID];
            const double mass = (!mc->getCharge()) ? mc->getMass() : 0;
            const int32_t pdg = (!mc->getCharge()) ? mc->getPDG() : 0;
            if (m_log->level() <= spdlog::level::debug) {
                if (mc->getCharge()) {
                    m_log->debug("   --> associated mcparticle is not a neutral (PDG: {}), "
                                 "setting the reconstructed particle ID to 0 (unidentified)", mc->getPDG());
                }
                m_log->debug("   --> found matching associated mcparticle with PDG: {}, energy: {}", pdg,
                             mc->getEnergy());
            }

            // Reconstruct our neutrals and add them to the list
            const auto outpart = reconstruct_neutral(clus, mass, pdg);
            m_log->debug(" --> Reconstructed neutral particle with PDG: {}, energy: {}", outpart.getPDG(),
                         outpart.getEnergy());

            outparts->push_back(outpart);

            // Create truth associations
            auto assoc = edm4eic::MutableMCRecoParticleAssociation();
            assoc.setRecID(outpart.getObjectID().index);
            assoc.setSimID(mcID);
            assoc.setWeight(1.0);
            assoc.setRec(outpart);
            //assoc.setSim(mcparticles[mcID]);
        }

        return {outparts, outpartsassoc};
    }



    // get a map of mcID --> cluster
    // input: cluster_collections --> list of handles to all cluster collections
    std::map<int, const edm4eic::Cluster*> MatchClusters::indexedClusters(
            const std::vector<std::vector<const edm4eic::Cluster*>> &cluster_collections,
            const std::vector<std::vector<const edm4eic::MCRecoClusterParticleAssociation*>> &associations_collections) {
        std::map<int, const edm4eic::Cluster*> matched = {};

        // loop over cluster collections
        for (const auto &clusters: cluster_collections) {

            // loop over clusters
            for (const auto &cluster: clusters) {

                int mcID = -1;

                // loop over association collections
                for (const auto &associations: associations_collections) {

                    // find associated particle
                    for (const auto &assoc: associations) {
                        if (assoc->getRec() == *cluster) {
                            mcID = assoc->getSimID();
                            break;
                        }
                    }

                    // found associated particle
                    if (mcID != -1) {
                        break;
                    }
                }

                m_log->trace(" --> Found cluster with mcID {} and energy {}", mcID, cluster->getEnergy());

                if (mcID < 0) {
                    m_log->trace("   --> WARNING: no valid MC truth link found, skipping cluster...");
                    continue;
                }

                const bool duplicate = matched.count(mcID);
                if (duplicate) {
                    m_log->trace("   --> WARNING: this is a duplicate mcID, keeping the higher energy cluster");

                    if (cluster->getEnergy() < matched[mcID]->getEnergy()) {
                        continue;
                    }
                }
                matched[mcID] = cluster;
            }
        }
        return matched;
    }


    // reconstruct a neutral cluster
    // (for now assuming the vertex is at (0,0,0))
    edm4eic::ReconstructedParticle MatchClusters::reconstruct_neutral(const edm4eic::Cluster *cluster, const double mass, const int32_t pdg) const {
        const float energy = cluster->getEnergy();
        const float p = energy < mass ? 0 : std::sqrt(energy * energy - mass * mass);
        const auto position = cluster->getPosition();
        const auto momentum = p * (position / edm4eic::magnitude(position));
        // setup our particle
        edm4eic::MutableReconstructedParticle part;
        part.setMomentum(momentum);
        part.setPDG(pdg);
        part.setCharge(0);
        part.setEnergy(energy);
        part.setMass(mass);
        return part;
    }



} // namespace Jug::Fast
