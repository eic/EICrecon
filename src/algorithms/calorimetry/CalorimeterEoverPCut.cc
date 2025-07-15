// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tomas Sosa, Wouter Deconinck

#include "CalorimeterEoverPCut.h"

#include <vector>                                      // for any use of std::vector
#include <gsl/pointers>                                // for gsl::not_null<>
#include <podio/RelationRange.h>                       // for cluster.getHits()

#include <edm4eic/ClusterCollection.h>                 // for ClusterCollection
#include <edm4eic/CalorimeterHitCollection.h>           // for CalorimeterHitCollection
#include <edm4eic/CalorimeterHit.h>                     // for getLayer()/getEnergy()
#include <edm4eic/TrackClusterMatchCollection.h>        // for TrackClusterMatchCollection
#include <edm4eic/TrackClusterMatch.h>                  // for TrackClusterMatch::makeEmpty()
#include <edm4eic/Track.h>                              // for Track.getMomentum()
#include <edm4hep/utils/vector_utils.h>                // for magnitude()

namespace eicrecon {

void CalorimeterEoverPCut::process(const CalorimeterEoverPCut::Input& input,
                                   const CalorimeterEoverPCut::Output& output) const {
  // Unpack all pointers
  const auto& [clusters_notnull, track_matches_notnull, hits_notnull] = input;
  auto const& clusters                                                = *clusters_notnull;
  auto const& track_matches                                           = *track_matches_notnull;

  auto& [pid_coll_ptr] = output;
  auto& pid_coll       = *pid_coll_ptr;

  for (auto const& cluster : clusters) {
    double energyInDepth = 0.0;
    for (auto const& hit : cluster.getHits()) {
      int layer = hit.getLayer();
      if (layer <= m_maxLayer) {
        energyInDepth += hit.getEnergy();
      }
    }

    bool found      = false;
    auto best_match = edm4eic::TrackClusterMatch::makeEmpty();
    for (auto const& match : track_matches) {
      if (match.getCluster() == cluster &&
          ((not best_match.isAvailable()) || (match.getWeight() > best_match.getWeight()))) {
        found      = true;
        best_match = match;
      }
    }
    if (!found) {
      warning("Can't find a match for the cluster. Skipping...");
      continue;
    }

    double ptrack = edm4hep::utils::magnitude(best_match.getTrack().getMomentum());
    double ep     = (ptrack > 0 ? energyInDepth / ptrack : 0.0);

    if (ep > m_ecut) {
      auto pid = pid_coll.create();
      pid.setType(0);
      pid.setPDG(11);
      pid.setAlgorithmType(0);
      pid.setLikelihood(ep);
    }
  }
}

} // namespace eicrecon
