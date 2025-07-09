#include <edm4eic/EDM4eicVersion.h>
#if EDM4EIC_VERSION_MAJOR >= 8

#include <edm4hep/utils/vector_utils.h>
#include <cmath>
#include "CalorimeterEoverPCut.h"

namespace eicrecon {

void CalorimeterEoverPCut::process(const CalorimeterEoverPCut::Input&  input,
                                   const CalorimeterEoverPCut::Output& output) const {
  const auto& [clusters, assoc_opt] = input;
  auto&       [pid_coll_ptr]        = output;

  if (!assoc_opt) {
    warning("[E/P Cut] no MC associations, skipping");
    return;
  }
  const auto& assocs = *assoc_opt;
  auto&        pid_coll = *pid_coll_ptr;

  // Loop clusters
  for (const auto& cluster : *clusters) {
    // pick the MC association with largest weight
    bool found = false;
    edm4eic::MCRecoClusterParticleAssociation bestAssoc;
    for (const auto& assoc : assocs) {
      if (assoc.getRec() == cluster &&
         (!found || assoc.getWeight() > bestAssoc.getWeight())) {
        found     = true;
        bestAssoc = assoc;
      }
    }
    if (!found) {
      warning("[E/P Cut] cluster w/o assoc, skipping");
      continue;
    }

    // true momentum and E/P
    double ptrack = edm4hep::utils::magnitude(bestAssoc.getSim().getMomentum());
    double ep     = (ptrack>0 ? cluster.getEnergy()/ptrack : 0.0);

    // apply cut
    if (ep > m_ecut) {
      auto pid = pid_coll.create();
      pid.setType(0);          // alg tag
      pid.setPDG(11);          // electron
      pid.setAlgorithmType(0); // user tag
      pid.setLikelihood(ep);   // store EP
      // Note: ParticleIDCollection does not support setReference()
    }
  }
}

} // namespace eicrecon

#endif // EDM4EIC_VERSION_MAJOR >= 8
