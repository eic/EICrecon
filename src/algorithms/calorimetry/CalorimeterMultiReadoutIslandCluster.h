// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Aiden Wu

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/ProtoClusterCollection.h>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <string_view>
#include <vector>

#include "CalorimeterIslandCluster.h"
#include "CalorimeterMultiReadoutIslandClusterConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using CalorimeterMultiReadoutIslandClusterAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::CalorimeterHitCollection>,
                          algorithms::Output<edm4eic::ProtoClusterCollection>>;

class CalorimeterMultiReadoutIslandCluster
    : public CalorimeterMultiReadoutIslandClusterAlgorithm,
      public WithPodConfig<CalorimeterMultiReadoutIslandClusterConfig> {
public:
  CalorimeterMultiReadoutIslandCluster(std::string_view name)
      : CalorimeterMultiReadoutIslandClusterAlgorithm{
            name, {"inputHitCollection"}, {"outputProtoClusterCollection"},
            "Island clustering across multiple readouts."} {}

  void init() final;
  void process(const Input&, const Output&) const final;

  // Update source collection IDs in configured readout order.
  void setSourceCollectionIDs(const std::vector<std::uint32_t>& collectionIDs);

private:

  // Source collection-to-readout mapping.
  std::map<std::uint32_t, std::size_t> m_collectionReadouts;

  // Neighbor-checking algorithms for each readout.
  std::vector<std::unique_ptr<CalorimeterIslandCluster>> m_islands;
};

} // namespace eicrecon
