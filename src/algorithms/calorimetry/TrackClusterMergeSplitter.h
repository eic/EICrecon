// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Derek Anderson

#ifndef EICRECON_TRACKCLUSTERMERGESPLITTER_H
#define EICRECON_TRACKCLUSTERMERGESPLITTER_H

#include <string>
#include <utility>
#include <algorithm>
#include <algorithms/algorithm.h>
#include <string_view>
// edm4eic types
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
      void init() final;
      void process (const Input&, const Output&) const final;

    private:

      /* TODO private methods will go here */

  };  // end TrackClusterMergeSplitter

}  // end eicrecon namespace

#endif

