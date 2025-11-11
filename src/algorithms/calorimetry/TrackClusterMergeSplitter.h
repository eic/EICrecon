// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson, Dmitry Kalinkin

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/CalorimeterHit.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/ProtoClusterCollection.h>
#include <edm4eic/TrackClusterMatchCollection.h>
#if EDM4EIC_VERSION_MAJOR >= 8 && EDM4EIC_VERSION_MINOR >= 4
#include <edm4eic/TrackProtoClusterMatchCollection.h>
#endif
#include <edm4eic/Track.h>
#include <edm4eic/TrackPoint.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <algorithm>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "TrackClusterMergeSplitterConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

// --------------------------------------------------------------------------
//! Algorithm input/output
// --------------------------------------------------------------------------
using TrackClusterMergeSplitterAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::TrackClusterMatchCollection, edm4eic::ClusterCollection,
                      edm4eic::TrackSegmentCollection>,
    algorithms::Output<edm4eic::ProtoClusterCollection,
#if EDM4EIC_VERSION_MAJOR >= 8 && EDM4EIC_VERSION_MINOR >= 4
                       edm4eic::TrackProtoClusterMatchCollection
#endif
                       >>;

// --------------------------------------------------------------------------
//! Track-Based Cluster Merger/Splitter
// --------------------------------------------------------------------------
/*! An algorithm which takes a collection of clusters, matches
 *  track projections, and then decides to merge or split those
 *  clusters based on average E/p from simulations.
 *
 *  Heavily inspired by Eur. Phys. J. C (2017) 77:466
 */
class TrackClusterMergeSplitter : public TrackClusterMergeSplitterAlgorithm,
                                  public WithPodConfig<TrackClusterMergeSplitterConfig> {

public:
  // --------------------------------------------------------------------------
  //! Comparator struct for object IDs
  // --------------------------------------------------------------------------
  /*! Organizes objects by their ObjectID's in decreasing collection
   *  ID first, and second by decreasing index second.
   */
  template <typename T> struct CompareObjectID {
    bool operator()(const T& lhs, const T& rhs) const {
      if (lhs.getObjectID().collectionID == rhs.getObjectID().collectionID) {
        return (lhs.getObjectID().index < rhs.getObjectID().index);
      } else {
        return (lhs.getObjectID().collectionID < rhs.getObjectID().collectionID);
      }
    }
  }; // end CompareObjectID

  ///! Specialization of comparator for clusters
  using CompareClust = CompareObjectID<edm4eic::Cluster>;

  ///! Specialization of comparator for hits
  using CompareHit = CompareObjectID<edm4eic::CalorimeterHit>;

  ///! Alias for vectors of track segments
  using VecSeg = std::vector<edm4eic::TrackSegment>;

  ///! Alias for vectors of mutable protoclusters
  using VecProto = std::vector<edm4eic::MutableProtoCluster>;

  ///! Alias for vectors of clusters
  using VecClust = std::vector<edm4eic::Cluster>;

  ///! Alias for sets of clusters
  using SetClust = std::set<edm4eic::Cluster, CompareClust>;

  ///! Alias for a map of clusters to the segments of matched tracks
  using MapToVecSeg = std::map<edm4eic::Cluster, VecSeg, CompareClust>;

  ///! Alias for a map of clusters onto clusters to merge
  using MapToVecClust = std::map<edm4eic::Cluster, VecClust, CompareClust>;

  ///! Alias for a map of hits onto their splitting weights
  using MapToWeight = std::map<edm4eic::CalorimeterHit, double, CompareHit>;

  ///! Alias for a vector of weight maps
  using VecWeights = std::vector<MapToWeight>;

  ///! Algorithm constructor
  TrackClusterMergeSplitter(std::string_view name) : TrackClusterMergeSplitterAlgorithm {
    name, {"InputTrackClusterMatches", "InputClusterCollection", "InputTrackProjections"},
#if EDM4EIC_VERSION_MAJOR >= 8
        {"OutputProtoClusterCollection", "OutputTrackProtoClusterMatches"},
#else
        {"OutputProtoClusterCollection"},
#endif
        "Merges or splits clusters based on tracks matched to them."} {}

  // public method
  void process(const Input&, const Output&) const final;

private:
  // private methods
  void merge_and_split_clusters(const VecClust& to_merge, const VecSeg& to_split,
                                VecProto& new_protos) const;
  void add_cluster_to_proto(const edm4eic::Cluster& clust, edm4eic::MutableProtoCluster& proto,
                            std::optional<MapToWeight> split_weights = std::nullopt) const;

}; // end TrackClusterMergeSplitter

} // namespace eicrecon
