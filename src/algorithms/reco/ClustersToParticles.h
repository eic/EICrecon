// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/EDM4eicVersion.h>
#include <optional>

#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
#include <edm4eic/MCRecoParticleLinkCollection.h>
#endif

#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using ClustersToParticlesAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::ClusterCollection,
                      std::optional<edm4eic::MCRecoClusterParticleAssociationCollection>>,
    algorithms::Output<edm4eic::ReconstructedParticleCollection,
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                       std::optional<edm4eic::MCRecoParticleLinkCollection>,
#endif
                       std::optional<edm4eic::MCRecoParticleAssociationCollection>>>;

class ClustersToParticles : public ClustersToParticlesAlgorithm,
                           public WithPodConfig<NoConfig> {

public:
  ClustersToParticles(std::string_view name)
      : ClustersToParticlesAlgorithm{name,
                                    {"inputClusters", "inputClusterAssociations"},
                                    {"outputReconstructedParticles",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                     "outputParticleLinks",
#endif
                                     "outputParticleAssociations"},
                                    "Convert clusters to neutral reconstructed particles."} {}

  void init() final;

  void process(const Input& input, const Output& output) const final;
};

} // namespace eicrecon
