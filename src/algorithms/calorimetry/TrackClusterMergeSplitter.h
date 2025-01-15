// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Derek Anderson

#pragma once

#include <DD4hep/Detector.h>
#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/EDM4eicVersion.h>
#if EDM4EIC_VERSION_MAJOR >= 8
#include <edm4eic/TrackClusterMatchCollection.h>
#endif
#include <edm4eic/TrackPoint.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4hep/Vector3f.h>
#include <podio/ObjectID.h>
#include <algorithm>
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
  //! Comparator struct for clusters
  // --------------------------------------------------------------------------
  /*! Organizes protoclusters by their ObjectID's in decreasing collection
   *  ID first, and second by decreasing index second.
   */
  struct CompareClust {

    bool operator() (const edm4eic::Cluster& lhs, const edm4eic::Cluster& rhs) const {
      if (lhs.getObjectID().collectionID == rhs.getObjectID().collectionID) {
        return (lhs.getObjectID().index < rhs.getObjectID().index);
      } else {
        return (lhs.getObjectID().collectionID < rhs.getObjectID().collectionID);
      }
    }

  };  // end CompareCluster



  // --------------------------------------------------------------------------
  //! Convenience types
  // --------------------------------------------------------------------------
  using VecProj = std::vector<edm4eic::TrackPoint>;
  using VecClust = std::vector<edm4eic::Cluster>;
  using SetClust = std::set<edm4eic::Cluster, CompareClust>;
  using MapToVecProj = std::map<edm4eic::Cluster, VecProj, CompareClust>;
  using MapToVecClust = std::map<edm4eic::Cluster, VecClust, CompareClust>;



  // --------------------------------------------------------------------------
  //! Algorithm input/output
  // --------------------------------------------------------------------------
  using TrackClusterMergeSplitterAlgorithm = algorithms::Algorithm<
    algorithms::Input<
      edm4eic::ClusterCollection,
      edm4eic::TrackSegmentCollection
    >,
    algorithms::Output<
#if EDM4EIC_VERSION_MAJOR >= 8
      edm4eic::ClusterCollection,
      edm4eic::TrackClusterMatchCollection
#else
      edm4eic::ClusterCollection
#endif
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
#if EDM4EIC_VERSION_MAJOR >= 8
          {"OutputProtoClusterCollection", "OutputTrackClusterMatches"},
#else
          {"OutputProtoClusterCollection"},
#endif
          "Merges or splits clusters based on tracks projected to them."
        } {}

      // public methods
      void init(const dd4hep::Detector* detector);
      void process (const Input&, const Output&) const final;

    private:

      // private methods
      void get_projections(const edm4eic::TrackSegmentCollection* projections, VecProj& relevant_projects) const;
      void match_clusters_to_tracks(const edm4eic::ClusterCollection* clusters, const VecProj& projections, MapToVecProj& matches) const;
      void merge_and_split_clusters(const VecClust& to_merge, const VecProj& to_split, edm4eic::ClusterCollection* out_clusters) const;
      edm4eic::MutableCluster merge_clusters(const VecClust& old_clusts) const;

      // calorimeter id
      int m_idCalo {0};

  };  // end TrackClusterMergeSplitter

}  // end eicrecon namespace
