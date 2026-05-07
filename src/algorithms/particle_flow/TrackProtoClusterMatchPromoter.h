// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/ProtoClusterCollection.h>
#include <edm4eic/TrackClusterMatchCollection.h>
#include <edm4eic/TrackProtoClusterMatchCollection.h>
#include <spdlog/spdlog.h>
#include <string>
#include <string_view>

#include "services/log/Log_service.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

// ----------------------------------------------------------------------------
//! Algorithm input/output
// ----------------------------------------------------------------------------
using TrackProtoClusterMatchPromoterAlgorithm = algorithms::Algorithm<
    typename algorithms::Input<edm4eic::TrackProtoClusterMatchCollection,
                               edm4eic::ProtoClusterCollection, edm4eic::ClusterCollection>,
    typename algorithms::Output<edm4eic::TrackClusterMatchCollection>>;

// ============================================================================
//! Track-Protocluster Match Promoter
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
            {"inputTrackProtoclusterMatches", "inputProtoclusters", "inputClusters"},
            {"outputTrackClusterMatches"},
            "Copies track-protocluster matches onto track-cluster matches"} {}

  // public method
  void process(const Input&, const Output&) const final;

}; // end TrackProtoClusterMatchPromoter

} // namespace eicrecon
