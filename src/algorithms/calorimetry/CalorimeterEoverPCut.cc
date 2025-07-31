// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tomas Sosa, Wouter Deconinck

#include "CalorimeterEoverPCut.h"

#include <edm4eic/Track.h>
#include <edm4hep/utils/vector_utils.h>
#include <podio/RelationRange.h>
#include <vector>

namespace eicrecon {

void CalorimeterEoverPCut::init() {
  // Nothing
}

void CalorimeterEoverPCut::process(const Input& input, const Output& output) const {
  const auto& [clusters_notnull, matches_notnull, hits_notnull] = input;
  auto const& clusters                                          = *clusters_notnull;
  auto const& matches                                           = *matches_notnull;

  auto& [out_clusters_notnull, out_matches_notnull, out_pids_notnull] = output;
  auto& out_clusters                                                  = *out_clusters_notnull;
  auto& out_matches                                                   = *out_matches_notnull;
  auto& out_pids                                                      = *out_pids_notnull;

  for (auto const& in_cl : clusters) {
    edm4eic::MutableCluster out_cl = in_cl.clone();
    out_clusters.push_back(out_cl);

    double energyInDepth = 0.0;
    for (auto const& hit : in_cl.getHits()) {
      if (hit.getLayer() <= m_maxLayer) {
        energyInDepth += hit.getEnergy();
      }
    }

    bool found_match                      = false;
    edm4eic::TrackClusterMatch best_match = edm4eic::TrackClusterMatch::makeEmpty();
    for (auto const& m : matches) {
      if (m.getCluster() == in_cl && (!found_match || m.getWeight() > best_match.getWeight())) {
        best_match  = m;
        found_match = true;
      }
    }
    if (!found_match) {
      warning("No TrackClusterMatch for this cluster; skipping PID.");
      continue;
    }

    for (auto const& m : matches) {
      if (m.getCluster() == in_cl) {
        auto out_m = m.clone();
        out_m.setCluster(out_cl);
        out_matches.push_back(out_m);
      }
    }

    double ptrack = edm4hep::utils::magnitude(best_match.getTrack().getMomentum());
    double ep     = (ptrack > 0.0 ? energyInDepth / ptrack : 0.0);

    if (ep > m_ecut) {
      out_cl.addToParticleIDs(out_pids.create(
          /* type= */ 0,
          /* PDG=  */ 11,
          /* algo= */ 0,
          /* like= */ static_cast<float>(ep)));
    }
  }
}

} // namespace eicrecon
