// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Dmitry Kalinkin

#pragma once

#include <algorithms/algorithm.h>
#include <optional>
#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"

namespace edm4eic {
class ClusterCollection;
}
namespace edm4eic {
class MCRecoClusterParticleAssociationCollection;
}
namespace edm4eic {
class TensorCollection;
}

namespace eicrecon {

using CalorimeterParticleIDPreMLAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::ClusterCollection,
                      std::optional<edm4eic::MCRecoClusterParticleAssociationCollection>>,
    algorithms::Output<edm4eic::TensorCollection, std::optional<edm4eic::TensorCollection>>>;

class CalorimeterParticleIDPreML : public CalorimeterParticleIDPreMLAlgorithm,
                                   public WithPodConfig<NoConfig> {

public:
  CalorimeterParticleIDPreML(std::string_view name)
      : CalorimeterParticleIDPreMLAlgorithm{
            name, {"inputClusters"}, {"outputFeatureTensor", "outputTargetTensor"}, ""} {}

  void init() final;
  void process(const Input&, const Output&) const final;
};

} // namespace eicrecon
