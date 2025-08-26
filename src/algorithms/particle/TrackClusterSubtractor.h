// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/TrackClusterMatchCollection.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <string>
#include <string_view>

#include "PFTools.h"
#include "TrackClusterSubtractorConfig.h"
#include "algorithms/interfaces/ParticleSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

// --------------------------------------------------------------------------
//! Algorithm input/output
// --------------------------------------------------------------------------
using TrackClusterSubtractorAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::TrackClusterMatchCollection, edm4eic::TrackSegmentCollection>,
    algorithms::Output<edm4eic::ClusterCollection, edm4eic::ClusterCollection,
                       edm4eic::TrackClusterMatchCollection>>;

// --------------------------------------------------------------------------
//! Track-Cluster Subtraction
// --------------------------------------------------------------------------
/*! An algorithm which takes a collection of clusters and their matched
   *  tracks, subtracts the sum of all tracks pointing to the cluster,
   *  and outputs the remnant cluster and their matched tracks.
   */
class TrackClusterSubtractor : public TrackClusterSubtractorAlgorithm,
                               public WithPodConfig<TrackClusterSubtractorConfig> {

public:
  // ctor
  TrackClusterSubtractor(std::string_view name)
      : TrackClusterSubtractorAlgorithm{name,
                                        {"inputTrackClusterMatches", "inputTrackProjections"},
                                        {"outputSubtractedClusterCollection",
                                         "outputRemnantClusterCollection",
                                         "outputTrackSubtractedClusterMatches"},
                                        "Subtracts energy of tracks pointing to clusters."} {}

  // public methods
  void init();
  void process(const Input&, const Output&) const final;

private:
  // private methods
  double sum_track_energy(const PFTools::VecSeg& projects) const;
  bool is_zero(const double difference) const;

  // services
  const algorithms::ParticleSvc& m_parSvc = algorithms::ParticleSvc::instance();

}; // end TrackClusterSubtractor

} // namespace eicrecon
