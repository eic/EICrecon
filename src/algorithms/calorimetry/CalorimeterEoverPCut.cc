#include <edm4eic/EDM4eicVersion.h>

#if EDM4EIC_VERSION_MAJOR >= 8
#include <cstddef>
#include <services/log/Log.h>
#include <edm4hep/MCParticle.h>
#include <edm4hep/utils/vector_utils.h>
#include <cmath>
#include "CalorimeterEoverPCut.h"

namespace eicrecon {

void CalorimeterEoverPCut::init() {
  // Nothing to do on init
}

void CalorimeterEoverPCut::process(const CalorimeterEoverPCut::Input&  input,
                                   const CalorimeterEoverPCut::Output& output) const {

  // Unpack inputs and outputs
  const auto& [clusters, assoc_opt] = input;
  auto&       [pid_coll]            = output;  // shared_ptr<ParticleIDCollection>

  if (!assoc_opt) {
    jwarn << "[E/P Cut] No MC associations provided, skipping" << jendl;
    return;
  }
  const auto& assocs = *assoc_opt;

  // Loop over every cluster
  for (const auto& cluster : *clusters) {
    // Find the MC association with highest weight
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
      jwarn << "[E/P Cut] Cluster without association, skipping" << jendl;
      continue;
    }

    // Compute track momentum and E/P
    double ptrack = edm4hep::utils::magnitude(bestAssoc.getSim().getMomentum());
    double ep     = (ptrack > 0. ? cluster.getEnergy() / ptrack : 0.0);

    // Apply the cut
    if (ep > config().ecut) {
      auto pid = pid_coll->create();
      pid.setType(0);            // algorithm-specific type
      pid.setPDG(11);            // PDG code for electron
      pid.setAlgorithmType(0);   // user-defined algorithm tag
      pid.setLikelihood(ep);     // store EP as likelihood
      pid.setReference(cluster); // link back to the cluster
    }
  }
}

} // namespace eicrecon
#endif