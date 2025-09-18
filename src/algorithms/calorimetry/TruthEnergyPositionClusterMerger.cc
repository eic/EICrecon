// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten

#include "algorithms/calorimetry/TruthEnergyPositionClusterMerger.h"

#include <Evaluator/DD4hepUnits.h>
#include <edm4eic/Cov3f.h>
#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <cmath>
#include <gsl/pointers>
#include <initializer_list>
#include <vector>

#include "algorithms/calorimetry/TruthEnergyPositionClusterMergerConfig.h"

namespace eicrecon {

void TruthEnergyPositionClusterMerger::process(const Input& input, const Output& output) const {

  const auto [mcparticles, energy_clus, energy_assoc, pos_clus, pos_assoc] = input;
  auto [merged_clus, merged_assoc]                                         = output;

  debug("Merging energy and position clusters for new event");

  if (energy_clus->size() == 0 && pos_clus->size() == 0) {
    debug("Nothing to do for this event, returning...");
    return;
  }

  debug("Step 0/2: Getting indexed list of clusters...");

  // get an indexed map of all clusters
  debug(" --> Indexing energy clusters");
  auto energyMap = indexedClusters(*energy_clus, *energy_assoc);
  trace(" --> Found these energy clusters:");
  for (const auto& [mcID, eclus] : energyMap) {
    trace("   --> energy cluster {}, mcID: {}, energy: {}", eclus.getObjectID().index, mcID,
          eclus.getEnergy());
  }
  debug(" --> Indexing position clusters");
  auto posMap = indexedClusters(*pos_clus, *pos_assoc);
  trace(" --> Found these position clusters:");
  for (const auto& [mcID, pclus] : posMap) {
    trace("   --> position cluster {}, mcID: {}, energy: {}", pclus.getObjectID().index, mcID,
          pclus.getEnergy());
  }

  // loop over all position clusters and match with energy clusters
  debug("Step 1/2: Matching all position clusters to the available energy clusters...");
  for (const auto& [mcID, pclus] : posMap) {

    debug(" --> Processing position cluster {}, mcID: {}, energy: {}", pclus.getObjectID().index,
          mcID, pclus.getEnergy());

    if (energyMap.count(mcID)) {

      const auto& eclus = energyMap[mcID];

      auto new_clus = merged_clus->create();
      new_clus.setEnergy(eclus.getEnergy());
      new_clus.setEnergyError(eclus.getEnergyError());
      new_clus.setType(m_cfg.clusterType);
      new_clus.setTime(pclus.getTime());
      new_clus.setNhits(pclus.getNhits() + eclus.getNhits());
      new_clus.setPosition(pclus.getPosition());
      new_clus.setPositionError(pclus.getPositionError());
      new_clus.addToClusters(pclus);
      new_clus.addToClusters(eclus);
      for (const auto& cl : {pclus, eclus}) {
        for (const auto& hit : cl.getHits()) {
          new_clus.addToHits(hit);
        }
        new_clus.addToSubdetectorEnergies(cl.getEnergy());
      }
      for (const auto& param : pclus.getShapeParameters()) {
        new_clus.addToShapeParameters(param);
      }
      debug("   --> Found matching energy cluster {}, energy: {}", eclus.getObjectID().index,
            eclus.getEnergy());
      debug("   --> Created new combined cluster {}, energy: {}", new_clus.getObjectID().index,
            new_clus.getEnergy());

      // set association
      auto clusterassoc = merged_assoc->create();
      clusterassoc.setRecID(new_clus.getObjectID().index);
      clusterassoc.setSimID(mcID);
      clusterassoc.setWeight(1.0);
      clusterassoc.setRec(new_clus);
      clusterassoc.setSim((*mcparticles)[mcID]);

      // erase the energy cluster from the map, so we can in the end account for all
      // remaining clusters
      energyMap.erase(mcID);
    } else {
      debug("   --> No matching energy cluster found, copying over position cluster");
      auto new_clus = pclus.clone();
      new_clus.addToClusters(pclus);
      new_clus.setType(m_cfg.clusterType);
      merged_clus->push_back(new_clus);

      // set association
      auto clusterassoc = merged_assoc->create();
      clusterassoc.setRecID(new_clus.getObjectID().index);
      clusterassoc.setSimID(mcID);
      clusterassoc.setWeight(1.0);
      clusterassoc.setRec(new_clus);
      clusterassoc.setSim((*mcparticles)[mcID]);
    }
  }
  // Collect remaining energy clusters. Use mc truth position for these clusters, as
  // they should really have a match in the position clusters (and if they don't it due
  // to a clustering error).
  debug("Step 2/2: Collecting remaining energy clusters...");
  for (const auto& [mcID, eclus] : energyMap) {
    const auto& mc   = (*mcparticles)[mcID];
    const auto& p    = mc.getMomentum();
    const auto phi   = std::atan2(p.y, p.x);
    const auto theta = std::atan2(std::hypot(p.x, p.y), p.z);

    auto new_clus = merged_clus->create();
    new_clus.setEnergy(eclus.getEnergy());
    new_clus.setEnergyError(eclus.getEnergyError());
    new_clus.setType(m_cfg.clusterType);
    new_clus.setTime(eclus.getTime());
    new_clus.setNhits(eclus.getNhits());
    // FIXME use nominal dd4hep::radius of 110cm, and use start vertex theta and phi
    new_clus.setPosition(
        edm4hep::utils::sphericalToVector(78.5 * dd4hep::cm / dd4hep::mm, theta, phi));
    new_clus.addToClusters(eclus);

    debug(" --> Processing energy cluster {}, mcID: {}, energy: {}", eclus.getObjectID().index,
          mcID, eclus.getEnergy());
    debug("   --> Created new 'combined' cluster {}, energy: {}", new_clus.getObjectID().index,
          new_clus.getEnergy());

    // set association
    auto clusterassoc = merged_assoc->create();
    clusterassoc.setRecID(new_clus.getObjectID().index);
    clusterassoc.setSimID(mcID);
    clusterassoc.setWeight(1.0);
    clusterassoc.setRec(new_clus);
    clusterassoc.setSim(mc);
  }
}

// get a map of MCParticle index --> cluster
// input: cluster_collections --> list of handles to all cluster collections
std::map<int, edm4eic::Cluster> TruthEnergyPositionClusterMerger::indexedClusters(
    const edm4eic::ClusterCollection& clusters,
    const edm4eic::MCRecoClusterParticleAssociationCollection& associations) const {

  std::map<int, edm4eic::Cluster> matched = {};

  for (const auto& cluster : clusters) {
    int mcID = -1;

    // find associated particle
    for (const auto& assoc : associations) {
      if (assoc.getRec() == cluster) {
        mcID = assoc.getSimID();
        break;
      }
    }

    trace(" --> Found cluster: {} with mcID {} and energy {}", cluster.getObjectID().index, mcID,
          cluster.getEnergy());

    if (mcID < 0) {
      trace("   --> WARNING: no valid MC truth link found, skipping cluster...");
      continue;
    }

    const bool duplicate = matched.count(mcID);
    if (duplicate) {
      trace("   --> WARNING: this is a duplicate mcID, keeping the higher energy cluster");
      if (cluster.getEnergy() < matched[mcID].getEnergy()) {
        continue;
      }
    }

    matched[mcID] = cluster;
  }
  return matched;
}

} // namespace eicrecon
