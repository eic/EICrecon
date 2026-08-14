// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tomas Sosa, Wouter Deconinck

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/TensorCollection.h>
#include <edm4hep/ParticleIDCollection.h>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using CalorimeterParticleIDBICPostMLAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::ClusterCollection, edm4eic::TensorCollection>,
    algorithms::Output<edm4eic::ClusterCollection, edm4hep::ParticleIDCollection>>;

class CalorimeterParticleIDBICPostML : public CalorimeterParticleIDBICPostMLAlgorithm,
                                       public WithPodConfig<NoConfig> {

public:
  CalorimeterParticleIDBICPostML(std::string_view name)
      : CalorimeterParticleIDBICPostMLAlgorithm{
            name,
            {"inputMergedClusters", "inputPredictionsTensor"},
            {"outputMergedClusters", "outputParticleIDs"},
            "Attach BIC ONNX outputs to E/p-selected energy-position merged clusters"} {}

  void init() final;
  void process(const Input&, const Output&) const final;
};

} // namespace eicrecon
