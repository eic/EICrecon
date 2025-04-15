// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2025, Simon Gardner

#pragma once

#include <DD4hep/Detector.h>
#include <DD4hep/Segmentations.h>
#include <Parsers/Primitives.h>
#include <algorithms/algorithm.h>
#include <edm4eic/Measurement2DCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <string>
#include <string_view>
#include <vector>

#include "FarDetectorTrackerClusterConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using FarDetectorTrackerClusterAlgorithm =
    algorithms::Algorithm<algorithms::Input<std::vector<edm4eic::TrackerHitCollection>>,
                          algorithms::Output<std::vector<edm4eic::Measurement2DCollection>>>;

class FarDetectorTrackerCluster : public FarDetectorTrackerClusterAlgorithm,
                                  public WithPodConfig<FarDetectorTrackerClusterConfig> {

public:
  FarDetectorTrackerCluster(std::string_view name)
      : FarDetectorTrackerClusterAlgorithm{name,
                                           {"inputHitCollection"},
                                           {"outputClusterPositionCollection"},
                                           "Simple weighted clustering of hits by x-y component of "
                                           "single detector element segmentation"} {}

  /** One time initialization **/
  void init() final;

  /** Event by event processing **/
  void process(const Input&, const Output&) const final;

  /** Cluster hits **/
  void ClusterHits(const edm4eic::TrackerHitCollection&, edm4eic::Measurement2DCollection&) const;

private:
  const dd4hep::Detector* m_detector{nullptr};
  const dd4hep::BitFieldCoder* m_id_dec{nullptr};
  dd4hep::Segmentation m_seg;

  int m_x_idx{0};
  int m_y_idx{0};
};

} // namespace eicrecon
