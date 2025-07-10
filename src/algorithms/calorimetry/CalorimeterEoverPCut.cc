// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tomas Sosa

#include <edm4eic/EDM4eicVersion.h>

#include <edm4hep/utils/vector_utils.h>
#include <cmath>
#include "CalorimeterEoverPCut.h"
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/CalorimeterHit.h>

namespace eicrecon {

void CalorimeterEoverPCut::process(const CalorimeterEoverPCut::Input& input,
                                   const CalorimeterEoverPCut::Output& output) const {
  // Unpack all three pointers
  const auto& [clusters_notnull, assocs_notnull, hits_notnull] = input;
  auto const& clusters = *clusters_notnull;
  auto const& assocs   = *assocs_notnull;
  auto const& hits     = *hits_notnull;

  auto&       [pid_coll_ptr] = output;
  auto&        pid_coll      = *pid_coll_ptr;

  for ( auto const& cluster : clusters ) {
    double energyInDepth = 0.0;
    for ( auto const& hit : cluster.getHits() ) {
      int layer = hit.getLayer();
      if ( layer <= m_maxLayer ) {
        energyInDepth += hit.getEnergy();
      }
    }

    bool found = false;
    edm4eic::MCRecoClusterParticleAssociation bestAssoc;
    for ( auto const& assoc : assocs ) {
      if ( assoc.getRec() == cluster
           && (!found || assoc.getWeight() > bestAssoc.getWeight()) ) {
        found     = true;
        bestAssoc = assoc;
      }
    }
    if ( !found ) {
      warning("[E/P Cut] cluster w/o assoc, skipping");
      continue;
    }

    double ptrack = edm4hep::utils::magnitude(bestAssoc.getSim().getMomentum());
    double ep     = (ptrack > 0 ? energyInDepth / ptrack : 0.0);

    if ( ep > m_ecut ) {
      auto pid = pid_coll.create();
      pid.setType(0);
      pid.setPDG(11);
      pid.setAlgorithmType(0);
      pid.setLikelihood(ep);
    }
  }
}



} // namespace eicrecon