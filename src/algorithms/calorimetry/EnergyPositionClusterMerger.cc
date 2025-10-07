// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten

#include "algorithms/calorimetry/EnergyPositionClusterMerger.h"

#include <edm4eic/Cov3f.h>
#include <edm4hep/MCParticle.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <podio/ObjectID.h>
#include <cmath>
#include <cstddef>
#include <gsl/pointers>
#include <limits>
#include <vector>

#include "algorithms/calorimetry/EnergyPositionClusterMergerConfig.h"

namespace eicrecon {

void EnergyPositionClusterMerger::process(const Input& input, const Output& output) const {

  const auto [energy_clus, energy_assoc, pos_clus, pos_assoc] = input;
  auto [merged_clus, merged_assoc]                            = output;

  debug("Merging energy and position clusters for new event");

  if (energy_clus->empty() && pos_clus->empty()) {
    debug("Nothing to do for this event, returning...");
    return;
  }

  std::vector<bool> consumed(energy_clus->size(), false);

  // use position clusters as starting point
  for (const auto& pc : *pos_clus) {

    trace(" --> Processing position cluster {}, energy: {}", pc.getObjectID().index,
          pc.getEnergy());

    // check if we find a good match
    int best_match    = -1;
    double best_delta = std::numeric_limits<double>::max();
    for (std::size_t ie = 0; ie < energy_clus->size(); ++ie) {
      if (consumed[ie]) {
        continue;
      }

      const auto& ec = (*energy_clus)[ie];

      trace("  --> Evaluating energy cluster {}, energy: {}", ec.getObjectID().index,
            ec.getEnergy());

      // 1. stop if not within tolerance
      //    (make sure to handle rollover of phi properly)
      const double de_rel = std::abs((pc.getEnergy() - ec.getEnergy()) / ec.getEnergy());
      const double deta =
          std::abs(edm4hep::utils::eta(pc.getPosition()) - edm4hep::utils::eta(ec.getPosition()));
      // check the tolerance for sin(dphi/2) to avoid the hemisphere problem and allow
      // for phi rollovers
      const double dphi = edm4hep::utils::angleAzimuthal(pc.getPosition()) -
                          edm4hep::utils::angleAzimuthal(ec.getPosition());
      const double dsphi = std::abs(sin(0.5 * dphi));
      if ((m_cfg.energyRelTolerance > 0 && de_rel > m_cfg.energyRelTolerance) ||
          (m_cfg.etaTolerance > 0 && deta > m_cfg.etaTolerance) ||
          (m_cfg.phiTolerance > 0 && dsphi > sin(0.5 * m_cfg.phiTolerance))) {
        continue;
      }
      // --> if we get here, we have a match within tolerance. Now treat the case
      //     where we have multiple matches. In this case take the one with the closest
      //     energies.
      // 2. best match?
      const double delta = std::abs(pc.getEnergy() - ec.getEnergy());
      if (delta < best_delta) {
        best_delta = delta;
        best_match = ie;
      }
    }

    // Create a merged cluster if we find a good match
    if (best_match >= 0) {

      const auto& ec = (*energy_clus)[best_match];

      auto new_clus = merged_clus->create();
      new_clus.setEnergy(ec.getEnergy());
      new_clus.setEnergyError(ec.getEnergyError());
      new_clus.setTime(pc.getTime());
      new_clus.setNhits(pc.getNhits() + ec.getNhits());
      new_clus.setPosition(pc.getPosition());
      new_clus.setPositionError(pc.getPositionError());
      new_clus.addToClusters(pc);
      new_clus.addToClusters(ec);

      trace("   --> Found matching energy cluster {}, energy: {}", ec.getObjectID().index,
            ec.getEnergy());
      trace("   --> Created a new combined cluster {}, energy: {}", new_clus.getObjectID().index,
            new_clus.getEnergy());

      // find association from energy cluster
      auto ea = energy_assoc->begin();
      for (; ea != energy_assoc->end(); ++ea) {
        if (ea->getRec() == ec) {
          break;
        }
      }
      // find association from position cluster if different
      auto pa = pos_assoc->begin();
      for (; pa != pos_assoc->end(); ++pa) {
        if (pa->getRec() == pc) {
          break;
        }
      }
      if (ea != energy_assoc->end() || pa != pos_assoc->end()) {
        // we must write an association
        if (ea != energy_assoc->end() && pa != pos_assoc->end()) {
          // we have two associations
          if (pa->getSimID() == ea->getSimID()) {
            // both associations agree on the MCParticles entry
            auto clusterassoc = merged_assoc->create();
            clusterassoc.setRecID(new_clus.getObjectID().index);
            clusterassoc.setSimID(ea->getSimID());
            clusterassoc.setWeight(1.0);
            clusterassoc.setRec(new_clus);
            clusterassoc.setSim(ea->getSim());
          } else {
            // both associations disagree on the MCParticles entry
            debug("   --> Two associations added to {} and {}", ea->getSimID(), pa->getSimID());
            auto clusterassoc1 = merged_assoc->create();
            clusterassoc1.setRecID(new_clus.getObjectID().index);
            clusterassoc1.setSimID(ea->getSimID());
            clusterassoc1.setWeight(0.5);
            clusterassoc1.setRec(new_clus);
            clusterassoc1.setSim(ea->getSim());
            auto clusterassoc2 = merged_assoc->create();
            clusterassoc2.setRecID(new_clus.getObjectID().index);
            clusterassoc2.setSimID(pa->getSimID());
            clusterassoc2.setWeight(0.5);
            clusterassoc2.setRec(new_clus);
            clusterassoc2.setSim(pa->getSim());
          }
        } else if (ea != energy_assoc->end()) {
          // no position association
          debug("   --> Only added energy cluster association to {}", ea->getSimID());
          auto clusterassoc = merged_assoc->create();
          clusterassoc.setRecID(new_clus.getObjectID().index);
          clusterassoc.setSimID(ea->getSimID());
          clusterassoc.setWeight(1.0);
          clusterassoc.setRec(new_clus);
          clusterassoc.setSim(ea->getSim());
        } else if (pa != pos_assoc->end()) {
          // no energy association
          debug("   --> Only added position cluster association to {}", pa->getSimID());
          auto clusterassoc = merged_assoc->create();
          clusterassoc.setRecID(new_clus.getObjectID().index);
          clusterassoc.setSimID(pa->getSimID());
          clusterassoc.setWeight(1.0);
          clusterassoc.setRec(new_clus);
          clusterassoc.setSim(pa->getSim());
        }
      }

      // label our energy cluster as consumed
      consumed[best_match] = true;

      debug("  Matched position cluster {} with energy cluster {}", pc.getObjectID().index,
            ec.getObjectID().index);
      debug("  - Position cluster: (E: {}, phi: {}, z: {})", pc.getEnergy(),
            edm4hep::utils::angleAzimuthal(pc.getPosition()), pc.getPosition().z);
      debug("  - Energy cluster: (E: {}, phi: {}, z: {})", ec.getEnergy(),
            edm4hep::utils::angleAzimuthal(ec.getPosition()), ec.getPosition().z);
      debug("  ---> Merged cluster: (E: {}, phi: {}, z: {})", new_clus.getEnergy(),
            edm4hep::utils::angleAzimuthal(new_clus.getPosition()), new_clus.getPosition().z);

    } else {

      debug("  Unmatched position cluster {}", pc.getObjectID().index);
    }
  }
}

} // namespace eicrecon
