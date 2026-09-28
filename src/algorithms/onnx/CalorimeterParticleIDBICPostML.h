// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tomas Sosa, Wouter Deconinck

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4eic/MCRecoClusterParticleLinkCollection.h>
#include <edm4eic/TensorCollection.h>
#include <edm4hep/ParticleIDCollection.h>
#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using CalorimeterParticleIDBICPostMLAlgorithm = algorithms::Algorithm<
    algorithms::Input<
        edm4eic::ClusterCollection, edm4eic::MCRecoClusterParticleAssociationCollection,
        edm4eic::ClusterCollection, edm4eic::ClusterCollection, edm4eic::ClusterCollection,
        edm4eic::ClusterCollection, edm4eic::TensorCollection>,
    algorithms::Output<edm4eic::ClusterCollection, edm4eic::MCRecoClusterParticleLinkCollection,
                       edm4eic::MCRecoClusterParticleAssociationCollection,
                       edm4hep::ParticleIDCollection>>;

class CalorimeterParticleIDBICPostML : public CalorimeterParticleIDBICPostMLAlgorithm,
                                       public WithPodConfig<NoConfig> {

public:
  CalorimeterParticleIDBICPostML(std::string_view name)
      : CalorimeterParticleIDBICPostMLAlgorithm{
            name,
            {"inputStandardClusters", "inputStandardClusterAssociations", "inputBICClusters",
             "inputImagingClusters", "inputStandardScFiClusters", "inputSelectedScFiClusters",
             "inputPredictionsTensor"},
            {"outputClusters", "outputClusterLinks", "outputClusterAssociations",
             "outputParticleIDs"},
            "Clone standard BEMC clusters and attach BIC ONNX PID to matched candidates"} {}

  void init() final;
  void process(const Input&, const Output&) const final;
};

} // namespace eicrecon
