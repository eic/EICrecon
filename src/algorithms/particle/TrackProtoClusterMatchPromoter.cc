// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson

#include <optional>

#include "TrackProtoClusterMatchPromoter.h"

namespace eicrecon {

// ----------------------------------------------------------------------------
//! Process inputs
// ----------------------------------------------------------------------------
/*! Promotes track-protocluster matches to track-cluster
 *  matches by <COPYING STUFF>
 *  ssuming the input cluster and protocluster
 *  collections are 1-to-1
 */
void TrackProtoClusterMatchPromoter::process(const TrackProtoClusterMatchPromoter::Input& input,
                                             const TrackProtoClusterMatchPromoter::Output& output) const {

  // grab inputs/outputs
  const auto [in_matches, in_protos, in_clusts] = input;
  auto [out_matches] = output;

  // exit if no matches in input collection
  if (in_matches->size() == 0) {
    debug("No track-protocluster matches in collection.");
    return;
  }

  // exit if protocluster/cluster collection
  // sizes are different
  //   --> 1-to-1 ordering can't be assumed!
  if (in_protos->size() != in_clusts->size()) {
    error("Number of input protoclusters ({}) not the same as number of input clusters ({})",
          in_protos->size(), in_clusts->size());
    return;
  }

  // loop through protoclusters
  //   --> if a protocluster has a match, create a
  //       match for the corresponding cluster
  for (std::size_t icl = 0; const auto& proto : *in_protos) {

    // NB track-protocluster matches are
    //   - FROM track
    //   - TO   protocluster
    std::optional<edm4eic::TrackProtoClusterMatch> match;
    for (const auto& pr_match : *in_matches) {
      if (pr_match.getTo() == proto) {
        match = pr_match;
        break;
      }
    }

    if (match.has_value()) {
      edm4eic::MutableTrackClusterMatch cl_match = out_matches->create();
      cl_match.setCluster((*in_clusts)[icl]);
      cl_match.setTrack(match.value().getFrom());
    }
    ++icl;
  }
}  // end 'process(Input&, Output&)'

} // namespace eicrecon
