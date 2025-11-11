// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Derek Anderson

#pragma once

#include <DD4hep/Detector.h>
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

  // specialization for clusters, hits
  using CompareClust = CompareObjectID<edm4eic::Cluster>;
  using CompareHit   = CompareObjectID<edm4eic::CalorimeterHit>;

  // --------------------------------------------------------------------------
  //! Convenience types
  // --------------------------------------------------------------------------
  // FIXME clean up this list when ready
  using VecTrk        = std::vector<edm4eic::Track>;
  using VecProj       = std::vector<edm4eic::TrackSegment>;
  using VecClust      = std::vector<edm4eic::Cluster>;
  using SetClust      = std::set<edm4eic::Cluster, CompareClust>;
  using MapToVecTrk   = std::map<edm4eic::Cluster, VecTrk, CompareClust>;
  using MapToVecProj  = std::map<edm4eic::Cluster, VecProj, CompareClust>;
  using MapToVecClust = std::map<edm4eic::Cluster, VecClust, CompareClust>;
  using MapToWeight   = std::map<edm4eic::CalorimeterHit, double, CompareHit>;
  using VecWeights    = std::vector<MapToWeight>;

  ///! Algorithm constructor
  TrackClusterMergeSplitter(std::string_view name) : TrackClusterMergeSplitterAlgorithm {
    name, {"InputTrackClusterMatches", "InputClusterCollection", "InputTrackProjections"},
#if EDM4EIC_VERSION_MAJOR >= 8
        {"OutputProtoClusterCollection", "OutputTrackProtoClusterMatches"},
#else
        {"OutputProtoClusterCollection"},
#endif
        "Merges or splits clusters based on tracks matched to them."
  }
  {}

  // public methods
  void init(const dd4hep::Detector* detector);
  void process(const Input&, const Output&) const final;

private:
  // private methods
  void merge_and_split_clusters(const VecClust& to_merge, const VecProj& to_split,
                                std::vector<edm4eic::MutableProtoCluster>& new_protos) const;
  void add_cluster_to_proto(const edm4eic::Cluster& clust, edm4eic::MutableProtoCluster& proto,
                            std::optional<MapToWeight> split_weights = std::nullopt) const;

  ///! System ID for calorimeter being processed
  unsigned int m_idCalo{0};

}; // end TrackClusterMergeSplitter

} // namespace eicrecon
