// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ePIC Collaboration

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <string>
#include <string_view>

#include <edm4eic/MCRecoParticleLinkCollection.h>

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/reco/ClustersToParticlesConfig.h"
#include "services/particle/ParticleSvc.h"

namespace eicrecon {

using ClustersToParticlesAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::ClusterCollection,
                                            edm4eic::MCRecoClusterParticleAssociationCollection>,
                          algorithms::Output<edm4eic::ReconstructedParticleCollection,
                                             edm4eic::MCRecoParticleLinkCollection,
                                             edm4eic::MCRecoParticleAssociationCollection>>;

class ClustersToParticles : public ClustersToParticlesAlgorithm,
                            public WithPodConfig<ClustersToParticlesConfig> {

public:
  ClustersToParticles(std::string_view name)
      : ClustersToParticlesAlgorithm{
            name,
            {"inputClusters", "inputClusterAssociations"},
            {"outputReconstructedParticles", "outputParticleLinks", "outputParticleAssociations"},
            "Convert clusters to neutral reconstructed particles."} {}

  void init() final;

  void process(const Input& input, const Output& output) const final;

private:
  const algorithms::ParticleSvc& m_particleSvc = algorithms::ParticleSvc::instance();
  double m_mass{0.0};
  int m_charge{0};
};

} // namespace eicrecon
