// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 - 2025, Simon Gardner

#pragma once

#include <Eigen/Core>
#include <algorithms/algorithm.h>
#include <algorithms/interfaces/WithPodConfig.h>
#include <DDRec/CellIDPositionConverter.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4eic/Measurement2DCollection.h>
#include <gsl/pointers>
#include <string>
#include <string_view>
#include <vector>

#include "FarDetectorLinearTrackingConfig.h"

namespace eicrecon {

using FarDetectorLinearTrackingAlgorithm =
    algorithms::Algorithm<algorithms::Input<std::vector<edm4eic::Measurement2DCollection>>,
                          algorithms::Output<edm4eic::TrackSegmentCollection>>;

class FarDetectorLinearTracking : public FarDetectorLinearTrackingAlgorithm,
                                  public WithPodConfig<FarDetectorLinearTrackingConfig> {

public:
  FarDetectorLinearTracking(std::string_view name)
      : FarDetectorLinearTrackingAlgorithm{name,
                                           {"inputHitCollections"},
                                           {"outputTrackSegments"},
                                           "Fit track segments from hits in the tracker layers"} {}

  /** One time initialization **/
  void init() final;

  /** Event by event processing **/
  void process(const Input&, const Output&) const final;

private:
  const dd4hep::rec::CellIDPositionConverter* m_cellid_converter{nullptr};
  Eigen::VectorXd m_layerWeights;

  Eigen::Vector3d m_optimumDirection;

  void buildMatrixRecursive(int level, Eigen::MatrixXd* hitMatrix,
                            const std::vector<std::vector<Eigen::Vector3d>>& hits,
                            gsl::not_null<edm4eic::TrackSegmentCollection*> outputTracks) const;

  void checkHitCombination(Eigen::MatrixXd* hitMatrix,
                           gsl::not_null<edm4eic::TrackSegmentCollection*> outputTracks) const;

  bool checkHitPair(const Eigen::Vector3d& hit1, const Eigen::Vector3d& hit2) const;

  /** Convert 2D clusters to 3D coordinates **/
  std::vector<Eigen::Vector3d>
  ConvertClusters(const edm4eic::Measurement2DCollection& clusters) const;
};

} // namespace eicrecon
