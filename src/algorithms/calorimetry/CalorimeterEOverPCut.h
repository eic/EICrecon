// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tomas Sosa, Wouter Deconinck

#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <algorithms/interfaces/WithPodConfig.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/TrackClusterMatchCollection.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4hep/ParticleIDCollection.h>
#include <DDSegmentation/BitFieldCoder.h>

#include "algorithms/calorimetry/CalorimeterEOverPCutConfig.h"

namespace eicrecon {

using CalorimeterEOverPCutAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::ClusterCollection, edm4eic::TrackClusterMatchCollection,
                      edm4eic::CalorimeterHitCollection>,
    algorithms::Output<edm4eic::ClusterCollection, edm4eic::TrackClusterMatchCollection,
                       edm4hep::ParticleIDCollection>>;

class CalorimeterEOverPCut : public CalorimeterEOverPCutAlgorithm,
                             public WithPodConfig<CalorimeterEOverPCutConfig> {
public:
  CalorimeterEOverPCut(std::string_view name)
      : CalorimeterEOverPCutAlgorithm{name,
                                      {"inputClusters", "inputTrackClusterMatches", "inputHits"},
                                      {"outputClusters", "outputTrackClusterMatches", "outputPIDs"},
                                      "E/p Cut with layer-depth limit"} {}

  void init() final;
  void process(const Input& input, const Output& output) const final;

private:
  const algorithms::GeoSvc& m_geo = algorithms::GeoSvc::instance();
  std::optional<std::pair<dd4hep::DDSegmentation::BitFieldCoder*, unsigned int>> m_id_dec_layer_idx;
};

} // namespace eicrecon
