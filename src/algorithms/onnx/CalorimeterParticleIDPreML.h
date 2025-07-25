// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Dmitry Kalinkin

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/TrackClusterMatchCollection.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4eic/TensorCollection.h>
#include <optional>
#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using CalorimeterParticleIDPreMLAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::ClusterCollection, edm4eic::TrackClusterMatchCollection,
                      std::optional<edm4eic::MCRecoClusterParticleAssociationCollection>>,
    algorithms::Output<edm4eic::TensorCollection, std::optional<edm4eic::TensorCollection>>>;

class CalorimeterParticleIDPreML : public CalorimeterParticleIDPreMLAlgorithm,
                                   public WithPodConfig<NoConfig> {

public:
  CalorimeterParticleIDPreML(std::string_view name)
      : CalorimeterParticleIDPreMLAlgorithm{name,
                                            {"inputClusters", "inputTrackClusterMatches"},
                                            {"outputFeatureTensor", "outputTargetTensor"},
                                            ""} {}

  void init() final;
  void process(const Input&, const Output&) const final;
};

} // namespace eicrecon
