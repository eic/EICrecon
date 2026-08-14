// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tomas Sosa, Wouter Deconinck

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/TensorCollection.h>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/onnx/CalorimeterParticleIDBICPreMLConfig.h"

namespace eicrecon {

using CalorimeterParticleIDBICPreMLAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::ClusterCollection,   // inputMergedClusters
                                            edm4eic::ClusterCollection,   // inputImagingClusters
                                            edm4eic::ClusterCollection>,  // inputScFiClusters
                          algorithms::Output<edm4eic::TensorCollection>>; // outputFeatureTensor

class CalorimeterParticleIDBICPreML : public CalorimeterParticleIDBICPreMLAlgorithm,
                                      public WithPodConfig<CalorimeterParticleIDBICPreMLConfig> {

public:
  CalorimeterParticleIDBICPreML(std::string_view name)
      : CalorimeterParticleIDBICPreMLAlgorithm{
            name,
            {"inputMergedClusters", "inputImagingClusters", "inputScFiClusters"},
            {"outputFeatureTensor"},
            "Build BIC CNN tensors from E/p-selected energy-position merged clusters"} {}

  void init() final;
  void process(const Input&, const Output&) const final;
};

} // namespace eicrecon
