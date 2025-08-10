// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Derek Anderson

#pragma once

#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <edm4eic/ProtoClusterCollection.h>
#include <edm4eic/TrackPoint.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4hep/Vector3f.h>
#include <podio/ObjectID.h>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "TrackClusterMergeSplitterConfig.h"
// for algorithm configuration
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

// --------------------------------------------------------------------------
//! Comparator struct for protoclusters
// --------------------------------------------------------------------------
/*! Organizes protoclusters by their ObjectID's in decreasing collection
   *  ID first, and second by decreasing index second.
   */
struct CompareProto {

  bool operator()(const edm4eic::ProtoCluster& lhs, const edm4eic::ProtoCluster& rhs) const {
    if (lhs.getObjectID().collectionID == rhs.getObjectID().collectionID) {
      return (lhs.getObjectID().index < rhs.getObjectID().index);
    } else {
      return (lhs.getObjectID().collectionID < rhs.getObjectID().collectionID);
    }
  }

}; // end CompareObjectID

// --------------------------------------------------------------------------
//! Convenience types
// --------------------------------------------------------------------------
using VecProj       = std::vector<edm4eic::TrackPoint>;
using VecClust      = std::vector<edm4eic::ProtoCluster>;
using SetClust      = std::set<edm4eic::ProtoCluster, CompareProto>;
using MapToVecProj  = std::map<edm4eic::ProtoCluster, VecProj, CompareProto>;
using MapToVecClust = std::map<edm4eic::ProtoCluster, VecClust, CompareProto>;

// --------------------------------------------------------------------------
//! Algorithm input/output
// --------------------------------------------------------------------------
using TrackClusterMergeSplitterAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::ProtoClusterCollection, edm4eic::TrackSegmentCollection>,
    algorithms::Output<edm4eic::ProtoClusterCollection>>;

// --------------------------------------------------------------------------
//! Track-Based Cluster Merger/Splitter
// --------------------------------------------------------------------------
/*! An algorithm which takes a collection of proto-clusters, matches
   *  track projections, and then decides to merge or split those proto-
   *  clusters based on average E/p from simulations.
   *
   *  Heavily inspired by Eur. Phys. J. C (2017) 77:466
   */
class TrackClusterMergeSplitter : public TrackClusterMergeSplitterAlgorithm,
                                  public WithPodConfig<TrackClusterMergeSplitterConfig> {

public:
  // ctor
  TrackClusterMergeSplitter(std::string_view name)
      : TrackClusterMergeSplitterAlgorithm{
            name,
            {"InputProtoClusterCollection", "InputTrackProjections"},
            {"OutputProtoClusterCollection"},
            "Merges or splits clusters based on tracks projected to them."} {}

  // public methods
  void init();
  void process(const Input&, const Output&) const final;

private:
  const algorithms::GeoSvc& m_geo = algorithms::GeoSvc::instance();

  // private methods
  void get_projections(const edm4eic::TrackSegmentCollection* projections,
                       VecProj& relevant_projects) const;
  void match_clusters_to_tracks(const edm4eic::ProtoClusterCollection* clusters,
                                const VecProj& projections, MapToVecProj& matches) const;
  void merge_and_split_clusters(const VecClust& to_merge, const VecProj& to_split,
                                edm4eic::ProtoClusterCollection* out_protoclusters) const;
  float get_cluster_energy(const edm4eic::ProtoCluster& clust) const;
  edm4hep::Vector3f get_cluster_position(const edm4eic::ProtoCluster& clust) const;

  // calorimeter id
  unsigned int m_idCalo{0};

}; // end TrackClusterMergeSplitter

} // namespace eicrecon
