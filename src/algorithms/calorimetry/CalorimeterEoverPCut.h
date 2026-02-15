// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tomas Sosa, Wouter Deconinck

#pragma once

#include <string>
#include <string_view>
#include <algorithms/algorithm.h>
#include <algorithms/interfaces/WithPodConfig.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/TrackClusterMatchCollection.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4hep/ParticleIDCollection.h>

#include "algorithms/calorimetry/CalorimeterEoverPCutConfig.h"

namespace eicrecon {

using CalorimeterEoverPCutAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::ClusterCollection, edm4eic::TrackClusterMatchCollection,
                      edm4eic::CalorimeterHitCollection>,
    algorithms::Output<edm4eic::ClusterCollection, edm4eic::TrackClusterMatchCollection,
                       edm4hep::ParticleIDCollection>>;

class CalorimeterEoverPCut : public CalorimeterEoverPCutAlgorithm,
                             public WithPodConfig<CalorimeterEoverPCutConfig> {
public:
  CalorimeterEoverPCut(std::string_view name)
      : CalorimeterEoverPCutAlgorithm{name,
                                      {"inputClusters", "inputTrackClusterMatches", "inputHits"},
                                      {"outputClusters", "outputTrackClusterMatches", "outputPIDs"},
                                      "E/P Cut with layer‑depth limit"} {}

  void init() final {};
  void process(const Input& input, const Output& output) const final;
};

} // namespace eicrecon
