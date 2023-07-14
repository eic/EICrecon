
// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten

#pragma once

#include <memory>
#include <random>

#include <spdlog/spdlog.h>

#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>

class CalorimeterClusterMerger {

protected:
    std::shared_ptr<spdlog::logger> m_log;

public:
    CalorimeterClusterMerger() = default;
    void AlgorithmInit(std::shared_ptr<spdlog::logger>& logger);
    void AlgorithmChangeRun();
    std::pair<std::unique_ptr<edm4eic::ClusterCollection>, std::unique_ptr<edm4eic::MCRecoClusterParticleAssociationCollection>> AlgorithmProcess(const edm4eic::ClusterCollection&, const edm4eic::MCRecoClusterParticleAssociationCollection&);

private:
// get a map of MCParticle index--> std::vector<Cluster> for clusters that belong together

  std::map<int, edm4eic::ClusterCollection> indexedClusterLists(const edm4eic::ClusterCollection &clusters, const edm4eic::MCRecoClusterParticleAssociationCollection &associations) const {
    std::map<int, edm4eic::ClusterCollection> matched = {};

    // loop over clusters
    for (auto cluster : clusters) {

      int mcID = -1;

      // find associated particle
      for (const auto& assoc : associations) {
        if( assoc.getRecID() == cluster.getObjectID().index ){
          mcID = assoc.getSimID();
          break;
        }
      }

      m_log->debug("Cluster {} has MC ID {} and energy", cluster.id(), mcID, cluster.getEnergy());

      if (mcID < 0) {
        m_log->warn("No valid MC truth link found, skipping cluster...");
        continue;
      }

      if (!matched.count(mcID)) {
        matched[mcID].setSubsetCollection();
      }
      matched[mcID].push_back(cluster);
    }
    return matched;
  }

};
