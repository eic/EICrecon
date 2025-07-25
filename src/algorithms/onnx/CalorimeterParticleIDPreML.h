// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Dmitry Kalinkin

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4hep/ParticleIDCollection.h>
#include <edm4eic/TensorCollection.h>
#include <optional>
#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using CalorimeterParticleIDPreMLAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::ClusterCollection,
                      std::optional<edm4eic::MCRecoClusterParticleAssociationCollection>,
                      edm4hep::ParticleIDCollection>,
    algorithms::Output<edm4eic::TensorCollection, std::optional<edm4eic::TensorCollection>>>;
class CalorimeterParticleIDPreML : public CalorimeterParticleIDPreMLAlgorithm,
                                   public WithPodConfig<NoConfig> {

public:
  CalorimeterParticleIDPreML(std::string_view name)
      : CalorimeterParticleIDPreMLAlgorithm{name,
                                            {"inputClusters", "inputClusterParticleAssociations",
                                             "inputParticleIDs"}, // ✱ tres inputs
                                            {"outputFeatureTensor", "outputTargetTensor"},
                                            ""} {}

  void init() final;
  void process(const Input&, const Output&) const final;
};

} // namespace eicrecon
