// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Derek Anderson

#ifndef EICRECON_TRACKCLUSTERMERGESPLITTER_H
#define EICRECON_TRACKCLUSTERMERGESPLITTER_H

// dd4hep utilities
#include <DD4hep/Detector.h>
#include <algorithms/algorithm.h>
#include <edm4eic/CalorimeterHit.h>
#include <edm4eic/ProtoClusterCollection.h>
// edm4eic types
#include <edm4eic/TrackPoint.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4hep/Vector3f.h>
#include <stddef.h>
#include <algorithm>
#include <map>
#include <string>
#include <string_view>
#include <vector>

// for algorithm configuration
#include "algorithms/interfaces/WithPodConfig.h"
#include "TrackClusterMergeSplitterConfig.h"



namespace eicrecon {

  // --------------------------------------------------------------------------
  //! Comparator struct for podio::ObjectID's
  // -------------------------------------------------------------------------
  /*! Organizes object ID's in decreasing collection ID first, and
   *  by decreasing index second.
   */
  struct CompareObjectID {

    bool operator() (const podio::ObjectID& lhs, const podio::ObjectID& rhs) const {
      return (lhs.collectionID == rhs.collectionID) ? (lhs.index < rhs.index)  : (lhs.collectionID < rhs.collectionID);
    }

  };  // end CompareObjectID



  // --------------------------------------------------------------------------
  //! Convenience types
  // --------------------------------------------------------------------------
  typedef std::map<podio::ObjectID, bool, CompareObjectID> MapToFlag;
  typedef std::map<podio::ObjectID, int, CompareObjectID> MapOneToIndex;
  typedef std::map<podio::ObjectID, std::vector<int>, CompareObjectID> MapProjToMerge;
  typedef std::map<podio::ObjectID, std::vector<podio::ObjectID>, CompareObjectID> MapClustToMerge;
  typedef std::vector<edm4eic::TrackPoint> VecTrkPoint;
  typedef std::vector<edm4eic::ProtoCluster> VecCluster;



  // --------------------------------------------------------------------------
  //! Algorithm input/output
  // --------------------------------------------------------------------------
  using TrackClusterMergeSplitterAlgorithm = algorithms::Algorithm<
    algorithms::Input<
      edm4eic::ProtoClusterCollection,
      edm4eic::TrackSegmentCollection
    >,
    algorithms::Output<
      edm4eic::ProtoClusterCollection
    >
  >;



  // --------------------------------------------------------------------------
  //! Track-Based Cluster Merger/Splitter
  // --------------------------------------------------------------------------
  /*! An algorithm which takes a collection of proto-clusters, matches
   *  track projections, and then decides to merge or split those proto-
   *  clusters based on average E/p from simulations.
   *
   *  Heavily inspired by Eur. Phys. J. C (2017) 77:466
   */
  class TrackClusterMergeSplitter :
    public TrackClusterMergeSplitterAlgorithm,
    public WithPodConfig<TrackClusterMergeSplitterConfig>
  {

    public:

      // ctor
      TrackClusterMergeSplitter(std::string_view name) :
        TrackClusterMergeSplitterAlgorithm {
          name,
          {"InputProtoClusterCollection", "InputTrackProjections"},
          {"OutputProtoClusterCollection"},
          "Merges or splits clusters based on tracks projected to them."
        } {}

      // public methods
      void init(const dd4hep::Detector* detector);
      void process (const Input&, const Output&) const final;

    private:

      // private methods
      void get_projections(const edm4eic::TrackSegmentCollection* projections, VecTrkPoint& relevant_projects) const;
      void match_clusters_to_tracks(const edm4eic::ProtoClusterCollection* clusters, const VecTrkPoint& projections, MapOneToIndex& matches) const;
      void merge_clusters(const edm4eic::TrackPoint& matched_trk, const VecCluster& to_merge, edm4eic::MutableProtoCluster& merged_clust) const;
      void copy_cluster(const edm4eic::ProtoCluster& old_clust, edm4eic::MutableProtoCluster& new_clust) const;
      float get_cluster_energy(const edm4eic::ProtoCluster& clust) const;
      edm4hep::Vector3f get_cluster_position(const edm4eic::ProtoCluster& clust) const;

      // additional services
      const dd4hep::Detector* m_detector {NULL};

      // calorimeter id
      int m_idCalo {0};

  };  // end TrackClusterMergeSplitter

}  // end eicrecon namespace

#endif
