// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2024, Sylvester Joosten, Chao Peng, Wouter Deconinck, David Lawrence, Derek Anderson

/*
 *  Reconstruct the cluster/layer info for imaging calorimeter
 *  Logarithmic weighting is used to describe energy deposit in transverse direction
 *
 *  Author: Chao Peng (ANL), 06/02/2021
 */

#pragma once

#include <Eigen/Dense>
#include <algorithm>

#include <algorithms/algorithm.h>
#include <DDRec/CellIDPositionConverter.h>
#include <DDRec/Surface.h>
#include <DDRec/SurfaceManager.h>

#include "algorithms/calorimetry/ClusterTypes.h"

// Event Model related classes
#include <edm4hep/MCParticleCollection.h>
#include <edm4eic/EDM4eicVersion.h>
#if EDM4EIC_VERSION_MAJOR >= 7
#include <edm4eic/MCRecoCalorimeterHitAssociationCollection.h>
#else
#include <edm4hep/SimCalorimeterHitCollection.h>
#endif
#include <edm4hep/utils/vector_utils.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4eic/ProtoClusterCollection.h>

#include "algorithms/interfaces/WithPodConfig.h"
#include "ImagingClusterRecoConfig.h"

namespace eicrecon {

using ImagingClusterRecoAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::ProtoClusterCollection,
#if EDM4EIC_VERSION_MAJOR >= 7
                                            edm4eic::MCRecoCalorimeterHitAssociationCollection
#else
                                            edm4hep::SimCalorimeterHitCollection
#endif
                                            >,
                          algorithms::Output<edm4eic::ClusterCollection,
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
  ImagingClusterReco(std::string_view name) : ImagingClusterRecoAlgorithm {
    name,
#if EDM4EIC_VERSION_MAJOR >= 7
        {"inputProtoClusterCollection", "mcRawHitAssocations"},
#else
        {"inputProtoClusterCollection", "mcHits"},
#endif
        {"outputClusterCollection", "outputClusterAssociations", "outputLayerCollection"},
        "Reconstruct the cluster/layer info for imaging calorimeter."
  }
  {}

public:
  void init() {}

  void process(const Input& input, const Output& output) const final;

private:
  static std::vector<edm4eic::MutableCluster>
  reconstruct_cluster_layers(const edm4eic::ProtoCluster& pcl);

  static edm4eic::MutableCluster
  reconstruct_layer(const std::vector<std::pair<const edm4eic::CalorimeterHit, float>>& hits);

  static edm4eic::MutableCluster reconstruct_cluster(const edm4eic::ProtoCluster& pcl);

  std::pair<double /* polar */, double /* azimuthal */>
  fit_track(const std::vector<edm4eic::MutableCluster>& layers) const;

  void associate_mc_particles(
      const edm4eic::Cluster& cl,
#if EDM4EIC_VERSION_MAJOR >= 7
      const edm4eic::MCRecoCalorimeterHitAssociationCollection* mchitassociations,
#else
      const edm4hep::SimCalorimeterHitCollection* mchits,
#endif
      edm4eic::MCRecoClusterParticleAssociationCollection* assocs) const;

  edm4hep::MCParticle get_primary(const edm4hep::CaloHitContribution& contrib) const;
};

} // namespace eicrecon
