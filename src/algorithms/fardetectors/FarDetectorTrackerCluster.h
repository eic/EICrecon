// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2024, Simon Gardner

#pragma once

#include <DD4hep/Detector.h>
#include <DD4hep/Segmentations.h>
#include <DDRec/CellIDPositionConverter.h>
#include <Parsers/Primitives.h>
#include <algorithms/algorithm.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4hep/TrackerHitCollection.h>
#include <podio/ObjectID.h>
#include <string>
#include <string_view>
#include <vector>

#include "FarDetectorTrackerClusterConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

// Cluster struct
struct FDTrackerCluster {
  unsigned long cellID{0};
  double x{0.0};
  double y{0.0};
  double energy{0.0};
  double time{0.0};
  double timeError{0.0};
  std::vector<podio::ObjectID> rawHits;
};
namespace eicrecon {

using FarDetectorTrackerClusterAlgorithm =
    algorithms::Algorithm<algorithms::Input<std::vector<edm4eic::RawTrackerHitCollection>>,
                          algorithms::Output<std::vector<edm4hep::TrackerHitCollection>>>;

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
  std::vector<FDTrackerCluster> ClusterHits(const edm4eic::RawTrackerHitCollection&) const;

  /** Convert clusters to TrackerHits **/
  void ConvertClusters(const std::vector<FDTrackerCluster>&, edm4hep::TrackerHitCollection&) const;

private:
  const dd4hep::Detector* m_detector{nullptr};
  const dd4hep::BitFieldCoder* m_id_dec{nullptr};
  const dd4hep::rec::CellIDPositionConverter* m_cellid_converter{nullptr};
  dd4hep::Segmentation m_seg;

  int m_x_idx{0};
  int m_y_idx{0};
};

} // namespace eicrecon
