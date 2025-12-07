// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 - 2025, Simon Gardner

#pragma once

#include <DDRec/CellIDPositionConverter.h>
#include <algorithms/algorithm.h>
#include <algorithms/interfaces/WithPodConfig.h>
#include <edm4eic/MCRecoTrackParticleAssociationCollection.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
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

namespace eicrecon {

using FarDetectorLinearTrackingAlgorithm = algorithms::Algorithm<
    algorithms::Input<std::vector<edm4eic::Measurement2DCollection>,
                      std::optional<edm4eic::MCRecoTrackerHitAssociationCollection>>,
    algorithms::Output<edm4eic::TrackCollection,
                       std::optional<edm4eic::MCRecoTrackParticleAssociationCollection>>>;

class FarDetectorLinearTracking : public FarDetectorLinearTrackingAlgorithm,
                                  public WithPodConfig<FarDetectorLinearTrackingConfig> {

public:
  FarDetectorLinearTracking(std::string_view name)
      : FarDetectorLinearTrackingAlgorithm{
            name,
            {"inputHitCollections", "inputMCRecoTrackerHitAssociations"},
            {"outputTrackCollection", "outputMCRecoTrackAssociations"},
            "Fit track segments from hits in the tracker layers"} {}

  /** One time initialization **/
  void init() final;

  /** Event by event processing **/
  void process(const Input&, const Output&) const final;

private:
  const dd4hep::rec::CellIDPositionConverter* m_cellid_converter{nullptr};
  Eigen::Matrix<double, 3, 1, Eigen::DontAlign> m_optimumDirection;
  Eigen::VectorXd m_layerWeights;

  void checkHitCombination(
      Eigen::MatrixXd* hitMatrix, edm4eic::TrackCollection* outputTracks,
      edm4eic::MCRecoTrackParticleAssociationCollection* assocTracks,
      const std::vector<gsl::not_null<const edm4eic::Measurement2DCollection*>>& inputHits,
      const std::vector<std::vector<edm4hep::MCParticle>>& assocParts,
      const std::vector<std::size_t>& layerHitIndex) const;

  /** Check if the last two hits are within a certain angle of the optimum direction **/
  bool checkHitPair(const Eigen::Vector3d& hit1, const Eigen::Vector3d& hit2) const;

  /** Convert 2D clusters to 3D coordinates and match associated particle **/
  void ConvertClusters(const edm4eic::Measurement2DCollection& clusters,
                       const edm4eic::MCRecoTrackerHitAssociationCollection& assoc_hits,
                       std::vector<std::vector<Eigen::Vector3d>>& pointPositions,
                       std::vector<std::vector<edm4hep::MCParticle>>& assoc_parts) const;
};

} // namespace eicrecon
