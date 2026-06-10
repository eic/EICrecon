// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tomas Sosa, Wouter Deconinck

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/TensorCollection.h>
#include <edm4hep/ParticleIDCollection.h>
#include <optional>
#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using CalorimeterParticleIDBICPreMLAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::ClusterCollection, std::optional<edm4hep::ParticleIDCollection>>,
    algorithms::Output<edm4eic::TensorCollection>>;

class CalorimeterParticleIDBICPreML : public CalorimeterParticleIDBICPreMLAlgorithm,
                                      public WithPodConfig<NoConfig> {

public:
  CalorimeterParticleIDBICPreML(std::string_view name)
      : CalorimeterParticleIDBICPreMLAlgorithm{name,
                                               {"inputClusters", "inputParticleIDs"},
                                               {"outputFeatureTensor"},
                                               "Build CNN feature tensor after E/p preselection"} {}

  void init() final;
  void process(const Input&, const Output&) const final;
};

} // namespace eicrecon
