// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#include <edm4eic/Track.h>
#include <podio/detail/LinkCollectionImpl.h>
#include <podio/detail/LinkCollectionIterator.h>
#include <cstddef>
#include <tuple>

#include "TrackProtoClusterMatchPromoter.h"

namespace eicrecon {

/*! For each track-protocluster match, create
 *  a corresponding track-cluster match.
 *
 *  \note Input protocluster and cluster collections
 *    are assumed to be 1-to-1, i.e. that the Nth
 *    cluster was reconstructed from the Nth
 *    protocluster.
 */
void TrackProtoClusterMatchPromoter::process(
    const TrackProtoClusterMatchPromoter::Input& input,
    const TrackProtoClusterMatchPromoter::Output& output) const {

  // grab inputs/outputs
  const auto [in_links, in_protos, in_clusts] = input;
  auto [out_matches]                          = output;

  // exit if no links in input collection
  if (in_links->size() == 0) {
    debug("No track-protocluster links in collection.");
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

  for (std::size_t icl = 0; const auto& proto : *in_protos) {
    for (const auto& pr_match : *in_links) {
      if (pr_match.getTo() == proto) {
        edm4eic::MutableTrackClusterMatch cl_match = out_matches->create();
        cl_match.setCluster((*in_clusts)[icl]);
        cl_match.setTrack(pr_match.getFrom());
        cl_match.setWeight(pr_match.getWeight());
      }
    }
    ++icl;
  }
} // end 'process(Input&, Output&)'
} // namespace eicrecon
