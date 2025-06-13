// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023-2025, Simon Gardner

#pragma once

// Event Model related classes
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/TrackCollection.h>
#include <Eigen/Core>
#include <string>
#include <string_view>

#include "algorithms/algorithm.h"
#include "algorithms/fardetectors/FarDetectorLinearProjectionConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using FarDetectorLinearProjectionAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::TrackCollection>,
                          algorithms::Output<edm4eic::TrackParametersCollection>>;

class FarDetectorLinearProjection : public FarDetectorLinearProjectionAlgorithm,
                                    public WithPodConfig<FarDetectorLinearProjectionConfig> {

public:
  FarDetectorLinearProjection(std::string_view name)
      : FarDetectorLinearProjectionAlgorithm{
            name, {"inputTrack"}, {"outputTrackParameters"}, "Project track segments to a plane"} {}

  /** One time initialization **/
  void init() final;

  /** Event by event processing **/
  void process(const Input&, const Output&) const final;

private:
  Eigen::Vector3d m_plane_position;
  Eigen::Vector3d m_plane_a;
  Eigen::Vector3d m_plane_b;
  Eigen::Matrix3d m_directions;
};

} // namespace eicrecon
