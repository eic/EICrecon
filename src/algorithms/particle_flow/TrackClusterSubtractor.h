// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/EDM4eicVersion.h>
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
#include <edm4eic/TrackClusterLinkCollection.h>
#endif
#include <edm4eic/TrackClusterMatchCollection.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "TrackClusterSubtractorConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"
#include "services/particle/ParticleSvc.h"

namespace eicrecon {

using TrackClusterSubtractorAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::TrackClusterMatchCollection, edm4eic::ClusterCollection,
                      edm4eic::TrackSegmentCollection>,
    algorithms::Output<edm4eic::ClusterCollection, edm4eic::ClusterCollection,
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                       edm4eic::TrackClusterLinkCollection,
#endif
                       edm4eic::TrackClusterMatchCollection>>;

// ==========================================================================
// Track-Cluster Subtraction
// ==========================================================================
/*! An algorithm which takes a collection of clusters and their matched
 *  tracks, subtracts the sum of all tracks pointing to the cluster,
 *  and outputs the remnant clusters, expected clusters, and their matched
 *  tracks.
 */
class TrackClusterSubtractor : public TrackClusterSubtractorAlgorithm,
                               public WithPodConfig<TrackClusterSubtractorConfig> {

public:
  ///! Algorithm constructor
  TrackClusterSubtractor(std::string_view name)
      : TrackClusterSubtractorAlgorithm{
            name,
            {"inputTrackClusterMatches", "inputClusters", "inputTrackProjections"},
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
            {"outputRemnantClusterCollection", "outputExpectedClusterCollection",
             "outputTrackExpectedClusterLinks", "outputTrackExpectedClusterMatches"},
#else
            {"outputRemnantClusterCollection", "outputExpectedClusterCollection",
             "outputTrackExpectedClusterMatches"},
#endif
            "Subtracts energy of tracks pointing to clusters."} {
  }

  void process(const Input&, const Output&) const final;

private:
  ///! Alias for vectors of track segments
  using segment_vector = std::vector<edm4eic::TrackSegment>;

  std::pair<double, double>
  sum_track_energy_and_covariance(const segment_vector& projections) const;
  bool is_track_energy_greater_than_calo(const double difference, const double variance) const;

  ///! Particle service instance for retrieving specified mass hypothesis
  const algorithms::ParticleSvc& m_parSvc = algorithms::ParticleSvc::instance();

}; // end TrackClusterSubtractor

} // namespace eicrecon
