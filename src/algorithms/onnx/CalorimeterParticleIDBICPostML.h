// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tomas Sosa, Wouter Deconinck

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/MCRecoClusterParticleLinkCollection.h>
#include <edm4eic/TensorCollection.h>
#include <edm4hep/ParticleIDCollection.h>
#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using CalorimeterParticleIDBICPostMLAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::ClusterCollection, edm4eic::MCRecoClusterParticleLinkCollection,
                      edm4eic::ClusterCollection, edm4eic::ClusterCollection,
                      edm4eic::ClusterCollection, edm4eic::ClusterCollection,
                      edm4eic::TensorCollection>,
    algorithms::Output<edm4eic::ClusterCollection, edm4eic::MCRecoClusterParticleLinkCollection,
                       edm4hep::ParticleIDCollection>>;

class CalorimeterParticleIDBICPostML : public CalorimeterParticleIDBICPostMLAlgorithm,
                                       public WithPodConfig<NoConfig> {

public:
  CalorimeterParticleIDBICPostML(std::string_view name)
      : CalorimeterParticleIDBICPostMLAlgorithm{
            name,
            {"inputStandardClusters", "inputStandardClusterLinks", "inputBICClusters",
             "inputImagingClusters", "inputStandardScFiClusters", "inputSelectedScFiClusters",
             "inputPredictionsTensor"},
            {"outputClusters", "outputClusterLinks", "outputParticleIDs"},
            "Clone the standard BEMC clusters matched to BIC candidates and attach the BIC ONNX "
            "PID"} {}

  void init() final;
  void process(const Input&, const Output&) const final;
};

} // namespace eicrecon
