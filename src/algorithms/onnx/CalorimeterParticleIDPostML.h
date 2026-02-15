// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Dmitry Kalinkin

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4eic/TensorCollection.h>
#include <edm4hep/ParticleIDCollection.h>
#include <optional>
#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"

#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
#include <edm4eic/MCRecoClusterParticleLinkCollection.h>
#endif

namespace eicrecon {

using CalorimeterParticleIDPostMLAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::ClusterCollection,
                      std::optional<edm4eic::MCRecoClusterParticleAssociationCollection>,
                      edm4eic::TensorCollection>,
    algorithms::Output<edm4eic::ClusterCollection,
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                       std::optional<edm4eic::MCRecoClusterParticleLinkCollection>,
#endif
                       std::optional<edm4eic::MCRecoClusterParticleAssociationCollection>,
                       edm4hep::ParticleIDCollection>>;

class CalorimeterParticleIDPostML : public CalorimeterParticleIDPostMLAlgorithm,
                                    public WithPodConfig<NoConfig> {

public:
  CalorimeterParticleIDPostML(std::string_view name)
      : CalorimeterParticleIDPostMLAlgorithm{
            name,
            {"inputClusters", "inputClusterAssociations", "inputPredictionsTensor"},
            {"outputClusters",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
             "outputClusterLinks",
#endif
             "outputClusterAssociations", "outputParticleIDs"},
            ""} {
  }

  void init() final;
  void process(const Input&, const Output&) const final;
};

} // namespace eicrecon
