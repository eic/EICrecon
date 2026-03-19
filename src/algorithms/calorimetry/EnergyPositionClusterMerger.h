// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <string>
#include <string_view>

#include "EnergyPositionClusterMergerConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
#include <edm4eic/MCRecoClusterParticleLinkCollection.h>
#endif

namespace eicrecon {

using EnergyPositionClusterMergerAlgorithm = algorithms::Algorithm<
    algorithms::Input<
        edm4eic::ClusterCollection, edm4eic::MCRecoClusterParticleAssociationCollection,
        edm4eic::ClusterCollection, edm4eic::MCRecoClusterParticleAssociationCollection>,
    algorithms::Output<edm4eic::ClusterCollection,
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                       edm4eic::MCRecoClusterParticleLinkCollection,
#endif
                       edm4eic::MCRecoClusterParticleAssociationCollection>>;

/** Simple algorithm to merge the energy measurement from cluster1 with the position
  * measurement of cluster2 (in case matching clusters are found). If not, it will
  * propagate the raw cluster from cluster1 or cluster2
  *
  * Matching occurs based on the cluster phi, eta and E variables, with tolerances
  * defined in the options file. A negative tolerance effectively disables
  * a check. The energy tolerance is defined as a relative number (e.g. 0.1)
  *
  * In case of ambiguity the closest cluster is merged.
  *
  * \ingroup reco
  */
class EnergyPositionClusterMerger : public EnergyPositionClusterMergerAlgorithm,
                                    public WithPodConfig<EnergyPositionClusterMergerConfig> {

public:
  EnergyPositionClusterMerger(std::string_view name)
      : EnergyPositionClusterMergerAlgorithm{
            name,
            {"energyClusterCollection", "energyClusterAssociations", "positionClusterCollection",
             "positionClusterAssociations"},
            {"outputClusterCollection",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
             "outputClusterLinks",
#endif
             "outputClusterAssociations"},
            "Merge energy and position clusters if matching."} {
  }

public:
  void init() {}

  void process(const Input& input, const Output& output) const final;
};

} // namespace eicrecon
