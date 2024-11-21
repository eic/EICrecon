// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Simon Gardner

#pragma once

#include <Eigen/Core>
#include <algorithms/algorithm.h>
#include <algorithms/interfaces/WithPodConfig.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4hep/EDM4hepVersion.h>
#if EDM4HEP_BUILD_VERSION < EDM4HEP_VERSION(0, 99, 0)
#include <edm4hep/TrackerHitCollection.h>
namespace edm4hep {
  using TrackerHit3DCollection = TrackerHitCollection;
}
#else
#include <edm4hep/TrackerHit3DCollection.h>
#endif
#include <gsl/pointers>
#include <string>
#include <string_view>
#include <vector>

#include "FarDetectorLinearTrackingConfig.h"

namespace eicrecon {

using FarDetectorLinearTrackingAlgorithm =
    algorithms::Algorithm<algorithms::Input<std::vector<edm4hep::TrackerHit3DCollection>>,
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
  Eigen::VectorXd m_layerWeights;

  Eigen::Vector3d m_optimumDirection;

  void
  buildMatrixRecursive(int level, Eigen::MatrixXd* hitMatrix,
                       const std::vector<gsl::not_null<const edm4hep::TrackerHit3DCollection*>>& hits,
                       gsl::not_null<edm4eic::TrackSegmentCollection*> outputTracks) const;

  void checkHitCombination(Eigen::MatrixXd* hitMatrix,
                           gsl::not_null<edm4eic::TrackSegmentCollection*> outputTracks) const;

  bool checkHitPair(const Eigen::Vector3d& hit1, const Eigen::Vector3d& hit2) const;
};

} // namespace eicrecon
