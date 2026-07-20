// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/ProtoClusterCollection.h>
#include <edm4eic/TrackClusterMatchCollection.h>
#include <edm4eic/TrackProtoClusterLinkCollection.h>
#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using TrackProtoClusterMatchPromoterAlgorithm = algorithms::Algorithm<
    typename algorithms::Input<edm4eic::TrackProtoClusterLinkCollection,
                               edm4eic::ProtoClusterCollection, edm4eic::ClusterCollection>,
    typename algorithms::Output<edm4eic::TrackClusterMatchCollection>>;

// ============================================================================
// Track-Protocluster Match Promoter
// ============================================================================
/*! An algorithm to promote track-protocluster matches to
 *  track-cluster matches. Assumes input cluster,
 *  protocluster collections are 1-to-1.
 */
class TrackProtoClusterMatchPromoter : public TrackProtoClusterMatchPromoterAlgorithm,
                                       public WithPodConfig<NoConfig> {

public:
  ///! Algorithm constructor
  TrackProtoClusterMatchPromoter(std::string_view name)
      : TrackProtoClusterMatchPromoterAlgorithm{
            name,
            {"inputTrackProtoclusterLinks", "inputProtoclusters", "inputClusters"},
            {"outputTrackClusterMatches"},
            "Copies track-protocluster matches onto track-cluster matches"} {}

  void process(const Input&, const Output&) const final;

}; // end TrackProtoClusterMatchPromoter

} // namespace eicrecon
