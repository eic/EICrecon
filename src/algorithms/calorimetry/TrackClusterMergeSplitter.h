// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson, Dmitry Kalinkin

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/CalorimeterHit.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/ProtoClusterCollection.h>
#include <edm4eic/TrackClusterMatchCollection.h>
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
#include <edm4eic/TrackProtoClusterLinkCollection.h>
#elif EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 4, 0)
#include <edm4eic/TrackProtoClusterMatchCollection.h>
#endif
#include <edm4eic/TrackSegmentCollection.h>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "TrackClusterMergeSplitterConfig.h"
#include "algorithms/interfaces/CompareObjectID.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using TrackClusterMergeSplitterAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::TrackClusterMatchCollection, edm4eic::ClusterCollection,
                      edm4eic::TrackSegmentCollection>,
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
    algorithms::Output<edm4eic::ProtoClusterCollection, edm4eic::TrackProtoClusterLinkCollection>>;
#elif EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 4, 0)
    algorithms::Output<edm4eic::ProtoClusterCollection, edm4eic::TrackProtoClusterMatchCollection>>;
#else
    algorithms::Output<edm4eic::ProtoClusterCollection>>;
#endif

// ==========================================================================
//! Track-Based Cluster Merger/Splitter
// ==========================================================================
/*! An algorithm which takes a collection of clusters, matches
 *  track projections, and then decides to merge or split those
 *  clusters based on average E/p from simulations.
 *
 *  Heavily inspired by Eur. Phys. J. C (2017) 77:466
 */
class TrackClusterMergeSplitter : public TrackClusterMergeSplitterAlgorithm,
                                  public WithPodConfig<TrackClusterMergeSplitterConfig> {

public:
  ///! Algorithm constructor
  TrackClusterMergeSplitter(std::string_view name)
      : TrackClusterMergeSplitterAlgorithm{
            name,
            {"InputTrackClusterMatches", "InputClusterCollection", "InputTrackProjections"},
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
            {"OutputProtoClusterCollection", "OutputTrackProtoClusterLinks"},
#elif EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 4, 0)
            {"OutputProtoClusterCollection", "OutputTrackProtoClusterMatches"},
#else
            {"OutputProtoClusterCollection"},
#endif
            "Merges or splits clusters based on tracks matched to them."} {
  }

  void process(const Input&, const Output&) const final;

private:
  ///! Alias for vectors of track segments
  using segment_vector = std::vector<edm4eic::TrackSegment>;

  ///! Alias for vectors of mutable protoclusters
  using protocluster_vector = std::vector<edm4eic::MutableProtoCluster>;

  ///! Alias for vectors of clusters
  using cluster_vector = std::vector<edm4eic::Cluster>;

  ///! Alias for a map of hits onto their splitting weights
  using hit_to_weight_map =
      std::map<edm4eic::CalorimeterHit, double, CompareObjectID<edm4eic::CalorimeterHit>>;

  void merge_and_split_clusters(const cluster_vector& to_merge, const segment_vector& to_split,
                                protocluster_vector& new_protos) const;
  static void add_cluster_to_proto(const edm4eic::Cluster& clust,
                                   edm4eic::MutableProtoCluster& proto,
                                   std::optional<hit_to_weight_map> split_weights = std::nullopt);

}; // end TrackClusterMergeSplitter
} // namespace eicrecon
