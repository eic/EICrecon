// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Derek Anderson

#ifndef EICRECON_TRACKCLUSTERMERGESPLITTER_H
#define EICRECON_TRACKCLUSTERMERGESPLITTER_H

#include <map>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <algorithms/algorithm.h>
#include <string_view>
// dd4hep utilities
#include <DD4hep/Detector.h>
#include <DDRec/CellIDPositionConverter.h>
// edm4eic types
#include <edm4eic/TrackPoint.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/TrackSegmentCollection.h>
// for algorithm configuration
#include "algorithms/interfaces/WithPodConfig.h"
#include "TrackClusterMergeSplitterConfig.h"



namespace eicrecon {

  using TrackClusterMergeSplitterAlgorithm = algorithms::Algorithm<
    algorithms::Input<
      edm4eic::ClusterCollection,
      edm4eic::TrackSegmentCollection
    >,
    algorithms::Output<
      edm4eic::ClusterCollection
    >
  >;

  // --------------------------------------------------------------------------
  //! Track-Based Cluster Merger/Splitter
  // --------------------------------------------------------------------------
  /*! An algorithm which takes a collection of reconstructed clusters,
   *  matches track projections, and then decides to merge or split
   *  those clusters based on average E/p from simulations.
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
          {"InputClusterCollection", "InputTrackProjections"},
          {"OutputClusterCollection"},
          "Merges or splits clusters based on tracks projected to them."
        } {}

      // public methods
      void init(const dd4hep::Detector* detector, const dd4hep::rec::CellIDPositionConverter* converter);
      void process (const Input&, const Output&) const final;

    private:

      // private methods
      void reset_bookkeepers() const;
      void get_projections(const edm4eic::TrackSegmentCollection* projections, const edm4eic::CalorimeterHit& cluster) const;
      void copy_cluster(const edm4eic::Cluster& old_clust, edm4eic::MutableCluster& new_clust) const;

      // additional services
      const dd4hep::Detector* m_detector {NULL};
      const dd4hep::rec::CellIDPositionConverter* m_converter {NULL};

      // bookkeeping members
      mutable std::map<int, bool> m_mapIsConsumed;
      mutable std::vector<edm4eic::TrackPoint> m_vecProject;

  };  // end TrackClusterMergeSplitter

}  // end eicrecon namespace

#endif

