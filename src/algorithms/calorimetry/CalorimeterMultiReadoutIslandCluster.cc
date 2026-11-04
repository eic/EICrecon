// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Aiden Wu

#include "CalorimeterMultiReadoutIslandCluster.h"

#include <edm4hep/utils/vector_utils.h>
#include <algorithm>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>

namespace eicrecon {

void CalorimeterMultiReadoutIslandCluster::init() {

  m_collectionReadouts.clear();
  m_islands.clear();
  std::set<std::string> readoutNames;

  // Configure readout adjacency rules.
  for (std::size_t i = 0; i < m_cfg.readouts.size(); ++i) {
    const auto& readout = m_cfg.readouts[i];
    const auto& cfg = readout.island;

    if (cfg.readout.empty() || !readoutNames.insert(cfg.readout).second) {
      throw std::runtime_error("Each multi-readout configuration needs a unique DD4hep readout");
    }

    // Associate source collections with this readout.
    for (auto collectionID : readout.collectionIDs) {
      if (!m_collectionReadouts.emplace(collectionID, i).second) {
        throw std::runtime_error("Duplicate multi-readout collection ID " +
                                 std::to_string(collectionID));
      }
    }

    // Use global distance if adjacency is unconfigured.
    if (cfg.adjacencyMatrix.empty() && cfg.localDistXY.empty() && cfg.localDistXZ.empty() &&
        cfg.localDistYZ.empty() && cfg.globalDistRPhi.empty() && cfg.globalDistEtaPhi.empty() &&
        cfg.dimScaledLocalDistXY.empty()) {
      m_islands.push_back(nullptr);
      continue;
    }

    // Initialize the readout's neighbor-checking algorithm.
    auto island = std::make_unique<CalorimeterIslandCluster>(cfg.readout + "MultiReadoutAdjacency");
    auto adjacencyConfig = cfg;
    adjacencyConfig.splitCluster = false;
    island->applyConfig(adjacencyConfig);
    island->init();
    m_islands.push_back(std::move(island));
  }
}

void CalorimeterMultiReadoutIslandCluster::setSourceCollectionIDs(
    const std::vector<std::uint32_t>& collectionIDs) {
  if (collectionIDs.size() != m_cfg.readouts.size()) {
    throw std::runtime_error("Expected one source collection ID per configured readout");
  }

  m_collectionReadouts.clear();
  for (std::size_t i = 0; i < collectionIDs.size(); ++i) {
    if (!m_collectionReadouts.emplace(collectionIDs[i], i).second) {
      throw std::runtime_error("Duplicate multi-readout source collection ID " +
                               std::to_string(collectionIDs[i]));
    }
  }
}

void CalorimeterMultiReadoutIslandCluster::process(const Input& input, const Output& output) const {
  const auto [hits] = input;
  auto [protoClusters] = output;

  std::vector<std::size_t> readouts;
  readouts.reserve(hits->size());
  std::vector<bool> visited(hits->size(), false);

  // Identify readouts and exclude hits below the energy threshold.
  for (std::size_t i = 0; i < hits->size(); ++i) {
    const auto hit = (*hits)[i];
    const auto collectionID = hit.getObjectID().collectionID;
    const auto entry = m_collectionReadouts.find(collectionID);

    if (entry == m_collectionReadouts.end()) {
      throw std::runtime_error("Unconfigured multi-readout collection ID " +
                               std::to_string(collectionID));
    }

    readouts.push_back(entry->second);
    visited[i] = hit.getEnergy() < m_cfg.readouts[entry->second].island.minClusterHitEdep;
  }

  // Group neighboring hits.
  for (std::size_t seed = 0; seed < hits->size(); ++seed) {

    if (visited[seed]) {
      continue;
    }

    std::vector<std::size_t> group{seed};
    visited[seed] = true;
    bool hasCenter = false;

    // Grow the group with breadth-first search.
    for (std::size_t next = 0; next < group.size(); ++next) {
      const auto index = group[next];
      const auto hit = (*hits)[index];
      const auto readout = readouts[index];

      if (hit.getEnergy() >= m_cfg.readouts[readout].island.minClusterCenterEdep) {
        hasCenter = true;
      }

      // Check neighbors.
      for (std::size_t other = 0; other < hits->size(); ++other) {
        if (visited[other]) {
          continue;
        }
        const auto otherHit = (*hits)[other];
        bool adjacent;

        // Use the shared readout's adjacency rule when configured; otherwise, use global distance.
        if (readout == readouts[other] && m_islands[readout]) {
          adjacent = m_islands[readout]->is_neighbour(hit, otherHit);
        } else {
          adjacent = m_cfg.globalDist > 0. &&
                     edm4hep::utils::magnitude(hit.getPosition() - otherHit.getPosition()) <=
                         m_cfg.globalDist;
        }

        if (adjacent) {
          visited[other] = true;
          group.push_back(other);
        }
      }
    }

    if (!hasCenter) {
      continue;
    }

    // Create a proto-cluster with hits in input order.
    std::sort(group.begin(), group.end());
    auto cluster = protoClusters->create();

    for (auto index : group) {
      cluster.addToHits((*hits)[index]);
      cluster.addToWeights(1.);
    }
  }
}

} // namespace eicrecon
