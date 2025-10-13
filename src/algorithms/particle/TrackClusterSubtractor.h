// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/EDM4eicVersion.h>
#include <podio/ObjectID.h>
#include <edm4eic/TrackClusterMatchCollection.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "TrackClusterSubtractorConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"
#include "services/particle/ParticleSvc.h"

namespace eicrecon {

// --------------------------------------------------------------------------
//! Algorithm input/output
// --------------------------------------------------------------------------
using TrackClusterSubtractorAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::TrackClusterMatchCollection, edm4eic::ClusterCollection,
                      edm4eic::TrackSegmentCollection>,
    algorithms::Output<edm4eic::ClusterCollection, edm4eic::ClusterCollection,
                       edm4eic::TrackClusterMatchCollection>>;

// ==========================================================================
//! Track-Cluster Subtraction
// ==========================================================================
/*! An algorithm which takes a collection of clusters and their matched
 *  tracks, subtracts the sum of all tracks pointing to the cluster,
 *  and outputs the remnant clusters, expected clusters, and their matched
 *  tracks.
 */
class TrackClusterSubtractor : public TrackClusterSubtractorAlgorithm,
                               public WithPodConfig<TrackClusterSubtractorConfig> {

public:
  // ------------------------------------------------------------------------
  //! Comparator struct for clusters
  // ------------------------------------------------------------------------
  /*! Organizes clusters by their ObjectIDs in decreasing collection
   *  ID first, and second by decreasing index second.
   */
  struct CompareClust {
    bool operator()(const edm4eic::Cluster& lhs, const edm4eic::Cluster& rhs) const {
      if (lhs.getObjectID().collectionID == rhs.getObjectID().collectionID) {
        return (lhs.getObjectID().index < rhs.getObjectID().index);
      } else {
        return (lhs.getObjectID().collectionID < rhs.getObjectID().collectionID);
      }
    }
  };

  ///! Alias for vectors of clusters
  using VecClust = std::vector<edm4eic::Cluster>;

  ///! Alias for vectors of track segments
  using VecSeg = std::vector<edm4eic::TrackSegment>;

  ///! Alias for a map from a cluster to the segments of matched tracks
  using MapToVecSeg = std::map<edm4eic::Cluster, VecSeg, CompareClust>;

  ///! Algorithm constructor
  TrackClusterSubtractor(std::string_view name)
      : TrackClusterSubtractorAlgorithm{
            name,
            {"inputTrackClusterMatches", "inputClusters", "inputTrackProjections"},
            {"outputRemnantClusterCollection", "outputExpectedClusterCollection",
             "outputTrackExpectedClusterMatches"},
            "Subtracts energy of tracks pointing to clusters."} {}

  // public method
  void process(const Input&, const Output&) const final;

private:
  // private methods
  double sum_track_energy(const VecSeg& projects) const;
  bool is_zero(const double difference) const;

  ///! Particle service instance for retrieving specified mass hypothesis
  const algorithms::ParticleSvc& m_parSvc = algorithms::ParticleSvc::instance();

}; // end TrackClusterSubtractor

} // namespace eicrecon
