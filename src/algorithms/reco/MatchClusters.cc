// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2024 Sylvester Joosten, Dmitry Romanov, Wouter Deconinck

// Takes a list of particles (presumed to be from tracking), and all available clusters.
// 1. Match clusters to their tracks using the mcID field
// 2. For unmatched clusters create neutrals and add to the particle list

#include <algorithms/logger.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <podio/ObjectID.h>
#include <cmath>
#include <gsl/pointers>
#include <map>

#include "MatchClusters.h"

namespace eicrecon {

void MatchClusters::process(const MatchClusters::Input& input,
                            const MatchClusters::Output& output) const {

  const auto [mcparticles, inparts, inpartsassoc, clusters, clustersassoc] = input;
  auto [outparts, outpartsassoc]                                           = output;

  debug("Processing cluster info for new event");

  debug("Step 0/2: Getting indexed list of clusters...");

  // get an indexed map of all clusters
  auto clusterMap = indexedClusters(clusters, clustersassoc);

  // 1. Loop over all tracks and link matched clusters where applicable
  // (removing matched clusters from the cluster maps)
  debug("Step 1/2: Matching clusters to charged particles...");

  for (const auto inpart : *inparts) {
    debug(" --> Processing charged particle {}, PDG {}, energy {}", inpart.getObjectID().index,
          inpart.getPDG(), inpart.getEnergy());

    auto outpart = inpart.clone();
    outparts->push_back(outpart);

    int mcID = -1;

    // find associated particle
    for (const auto& assoc : *inpartsassoc) {
      if (assoc.getRec().getObjectID() == inpart.getObjectID()) {
        mcID = assoc.getSim().getObjectID().index;
        break;
      }
    }

    trace("    --> Found particle with mcID {}", mcID);

    if (mcID < 0) {
      debug("    --> cannot match track without associated mcID");
      continue;
    }

    if (clusterMap.contains(mcID)) {
      const auto& clus = clusterMap[mcID];
      debug("    --> found matching cluster with energy: {}", clus.getEnergy());
      debug("    --> adding cluster to reconstructed particle");
      outpart.addToClusters(clus);
      clusterMap.erase(mcID);
    }

    // create truth associations
    auto assoc = outpartsassoc->create();
    assoc.setRecID(outpart.getObjectID().index);
    assoc.setSimID(mcID);
    assoc.setWeight(1.0);
    assoc.setRec(outpart);
    assoc.setSim((*mcparticles)[mcID]);
  }

  // 2. Now loop over all remaining clusters and add neutrals. Also add in Hcal energy
  // if a matching cluster is available
  debug("Step 2/2: Creating neutrals for remaining clusters...");
  for (const auto& [mcID, clus] : clusterMap) {
    debug(" --> Processing unmatched cluster with energy: {}", clus.getEnergy());

    // get mass/PDG from mcparticles, 0 (unidentified) in case the matched particle is charged.
    const auto mc     = (*mcparticles)[mcID];
    const double mass = 0.;
    const int32_t pdg = 0;
    if (level() <= algorithms::LogLevel::kDebug) {
      if (mc.getCharge() != 0.0F) {
        debug("   --> associated mcparticle is not a neutral (PDG: {}), "
              "setting the reconstructed particle ID to 0 (unidentified)",
              mc.getPDG());
      }
      debug("   --> found matching associated mcparticle with PDG: {}, energy: {}", pdg,
            mc.getEnergy());
    }

    // Reconstruct our neutrals and add them to the list
    const auto outpart = reconstruct_neutral(&clus, mass, pdg);
    debug(" --> Reconstructed neutral particle with PDG: {}, energy: {}", outpart.getPDG(),
          outpart.getEnergy());

    outparts->push_back(outpart);

    // Create truth associations
    auto assoc = outpartsassoc->create();
    assoc.setRecID(outpart.getObjectID().index);
    assoc.setSimID(mcID);
    assoc.setWeight(1.0);
    assoc.setRec(outpart);
    assoc.setSim((*mcparticles)[mcID]);
  }
}

// get a map of mcID --> cluster
// input: clusters --> all clusters
std::map<int, edm4eic::Cluster> MatchClusters::indexedClusters(
    const edm4eic::ClusterCollection* clusters,
    const edm4eic::MCRecoClusterParticleAssociationCollection* associations) const {

  // temporary map: mcID -> (cluster, weight) so we can choose the cluster with highest weight per mcID
  std::map<int, std::pair<edm4eic::Cluster, float>> bestForMc;
  

  // loop over clusters
  for (const auto cluster : *clusters) {

    int bestMcID = -1;
    float bestWeight = -1.f;

    // find best associated MC particle for this cluster (largest association weight)
    for (const auto assoc : *associations) {
      if (assoc.getRec() == cluster) {
        const int candMcID = assoc.getSim().getObjectID().index;
        const float w = assoc.getWeight();
        if (w > bestWeight) {
          bestWeight = w;
          bestMcID = candMcID;
        }
      }
    }

    trace(" --> Found cluster with best mcID {} weight {} and energy {}", bestMcID, bestWeight,
          cluster.getEnergy());

    if (bestMcID < 0) {
      trace("   --> WARNING: no valid MC truth link found, skipping cluster...");
      continue;
    }

    // For this mcID, keep the cluster with the highest association weight (tie-break by energy).
    auto it = bestForMc.find(bestMcID);
    if (it == bestForMc.end()) {
      bestForMc.emplace(bestMcID, std::make_pair(cluster, bestWeight));
    } else {
      const float existingWeight = it->second.second;
      if (bestWeight > existingWeight ||
          (bestWeight == existingWeight && cluster.getEnergy() > it->second.first.getEnergy())) {
        it->second = std::make_pair(cluster, bestWeight);
      }
    }
  }

  // Convert to the old API: map<int, edm4eic::Cluster>
  std::map<int, edm4eic::Cluster> matched;
  for (const auto& kv : bestForMc) {
    matched.emplace(kv.first, kv.second.first);
  }

  return matched;
}

// reconstruct a neutral cluster
// (for now assuming the vertex is at (0,0,0))
edm4eic::MutableReconstructedParticle
MatchClusters::reconstruct_neutral(const edm4eic::Cluster* cluster, const double mass,
                                   const int32_t pdg) {

  const float energy  = cluster->getEnergy();
  const float p       = energy < mass ? 0 : std::sqrt(energy * energy - mass * mass);
  const auto position = cluster->getPosition();
  const auto momentum = p * (position / edm4hep::utils::magnitude(position));
  // setup our particle
  edm4eic::MutableReconstructedParticle part;
  part.setMomentum(momentum);
  part.setPDG(pdg);
  part.setCharge(0);
  part.setEnergy(energy);
  part.setMass(mass);
  part.addToClusters(*cluster);
  return part;
}

} // namespace eicrecon
