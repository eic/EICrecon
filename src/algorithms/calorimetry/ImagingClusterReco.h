// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2024, Sylvester Joosten, Chao Peng, Wouter Deconinck, David Lawrence, Derek Anderson

/*
 *  Reconstruct the cluster/layer info for imaging calorimeter
 *  Logarithmic weighting is used to describe energy deposit in transverse direction
 *
 *  Author: Chao Peng (ANL), 06/02/2021
 */

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/MCRecoCalorimeterHitAssociationCollection.h>
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
#include <edm4eic/MCRecoCalorimeterHitLinkCollection.h>
#include <podio/LinkNavigator.h>
#endif
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4eic/ProtoClusterCollection.h>
#include <edm4hep/CaloHitContribution.h>
// Event Model related classes
#include <edm4hep/MCParticleCollection.h>
#include <iterator>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "ImagingClusterRecoConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
#include <edm4eic/MCRecoClusterParticleLinkCollection.h>
#endif

namespace eicrecon {

using ImagingClusterRecoAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::ProtoClusterCollection,
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                            edm4eic::MCRecoCalorimeterHitLinkCollection,
#endif
                                            edm4eic::MCRecoCalorimeterHitAssociationCollection>,
                          algorithms::Output<edm4eic::ClusterCollection,
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                             edm4eic::MCRecoClusterParticleLinkCollection,
#endif
                                             edm4eic::MCRecoClusterParticleAssociationCollection,
                                             edm4eic::ClusterCollection>>;

/** Imaging cluster reconstruction.
   *
   *  Reconstruct the cluster/layer info for imaging calorimeter
   *  Logarithmic weighting is used to describe energy deposit in transverse direction
   *
   *  \ingroup reco
   */
class ImagingClusterReco : public ImagingClusterRecoAlgorithm,
                           public WithPodConfig<ImagingClusterRecoConfig> {

public:
  ImagingClusterReco(std::string_view name)
      : ImagingClusterRecoAlgorithm{name,
                                    {"inputProtoClusterCollection",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                     "mcRawHitLinks",
#endif
                                     "mcRawHitAssocations"},
                                    {"outputClusterCollection",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                     "outputClusterLinks",
#endif
                                     "outputClusterAssociations", "outputLayerCollection"},
                                    "Reconstruct the cluster/layer info for imaging calorimeter."} {
  }

public:
  void init() {}

  void process(const Input& input, const Output& output) const final;

private:
  std::vector<edm4eic::MutableCluster>
  reconstruct_cluster_layers(const edm4eic::ProtoCluster& pcl) const;

  edm4eic::MutableCluster
  reconstruct_layer(const std::vector<std::pair<const edm4eic::CalorimeterHit, float>>& hits) const;

  edm4eic::MutableCluster reconstruct_cluster(const edm4eic::ProtoCluster& pcl) const;

  std::pair<double /* polar */, double /* azimuthal */>
  fit_track(const std::vector<edm4eic::MutableCluster>& layers) const;

  void associate_mc_particles(
      const edm4eic::Cluster& cl,
      const edm4eic::MCRecoCalorimeterHitAssociationCollection* mchitassociations,
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
      const podio::LinkNavigator* link_nav,
      edm4eic::MCRecoClusterParticleLinkCollection* links,
#endif
      edm4eic::MCRecoClusterParticleAssociationCollection* assocs) const;

  edm4hep::MCParticle get_primary(const edm4hep::CaloHitContribution& contrib) const;
};

} // namespace eicrecon
