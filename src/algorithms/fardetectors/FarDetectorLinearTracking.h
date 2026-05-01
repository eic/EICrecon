// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 - 2025, Simon Gardner

#pragma once

#include <DDRec/CellIDPositionConverter.h>
#include <algorithms/algorithm.h>
#include <algorithms/interfaces/WithPodConfig.h>
#include <edm4eic/MCRecoTrackParticleAssociationCollection.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
#include <edm4eic/MCRecoTrackerHitLinkCollection.h>
#include <podio/LinkNavigator.h>
#endif
#include <edm4eic/Measurement2DCollection.h>
#include <edm4eic/TrackCollection.h>
#include <edm4hep/MCParticle.h>
#include <Eigen/Core>
#include <cstddef>
#include <gsl/pointers>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "FarDetectorLinearTrackingConfig.h"

#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
#include <edm4eic/MCRecoTrackParticleLinkCollection.h>
#endif

namespace eicrecon {

using FarDetectorLinearTrackingAlgorithm = algorithms::Algorithm<
    algorithms::Input<std::vector<edm4eic::Measurement2DCollection>,
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                      std::optional<edm4eic::MCRecoTrackerHitLinkCollection>,
#endif
                      std::optional<edm4eic::MCRecoTrackerHitAssociationCollection>>,
    algorithms::Output<edm4eic::TrackCollection,
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                       std::optional<edm4eic::MCRecoTrackParticleLinkCollection>,
#endif
                       std::optional<edm4eic::MCRecoTrackParticleAssociationCollection>>>;

class FarDetectorLinearTracking : public FarDetectorLinearTrackingAlgorithm,
                                  public WithPodConfig<FarDetectorLinearTrackingConfig> {

public:
  FarDetectorLinearTracking(std::string_view name)
      : FarDetectorLinearTrackingAlgorithm{name,
                                           {"inputHitCollections",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                            "inputMCRecoTrackerHitLinks",
#endif
                                            "inputMCRecoTrackerHitAssociations"},
                                           {"outputTrackCollection",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                            "outputMCRecoTrackLinks",
#endif
                                            "outputMCRecoTrackAssociations"},
                                           "Fit track segments from hits in the tracker layers"} {
  }

  /** One time initialization **/
  void init() final;

  /** Event by event processing **/
  void process(const Input&, const Output&) const final;

private:
  const dd4hep::rec::CellIDPositionConverter* m_cellid_converter{nullptr};
  Eigen::VectorXd m_layerWeights;

  Eigen::Vector3d m_optimumDirection;

  void checkHitCombination(
      Eigen::MatrixXd* hitMatrix, edm4eic::TrackCollection* outputTracks,
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
      edm4eic::MCRecoTrackParticleLinkCollection* trackLinks,
#endif
      edm4eic::MCRecoTrackParticleAssociationCollection* assocTracks,
      const std::vector<gsl::not_null<const edm4eic::Measurement2DCollection*>>& inputHits,
      const std::vector<std::vector<edm4hep::MCParticle>>& assocParts,
      const std::vector<std::size_t>& layerHitIndex) const;

  /** Check if the last two hits are within a certain angle of the optimum direction **/
  bool checkHitPair(const Eigen::Vector3d& hit1, const Eigen::Vector3d& hit2) const;

  /** Convert 2D clusters to 3D coordinates and match associated particle **/
  void
  ConvertClusters(const edm4eic::Measurement2DCollection& clusters,
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                  const podio::LinkNavigator<edm4eic::MCRecoTrackerHitLinkCollection>& link_nav,
#endif
                  const edm4eic::MCRecoTrackerHitAssociationCollection& assoc_hits,
                  std::vector<std::vector<Eigen::Vector3d>>& pointPositions,
                  std::vector<std::vector<edm4hep::MCParticle>>& assoc_parts) const;
};

} // namespace eicrecon
