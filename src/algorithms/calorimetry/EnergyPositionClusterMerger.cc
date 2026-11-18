// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten

#include <edm4hep/MCParticle.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <podio/detail/Link.h>
#include <podio/detail/LinkCollectionImpl.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <initializer_list>
#include <limits>
#include <memory>
#include <tuple>
#include <vector>

#include "algorithms/calorimetry/EnergyPositionClusterMerger.h"
#include "algorithms/calorimetry/EnergyPositionClusterMergerConfig.h"
#include "algorithms/interfaces/LinkTruthUtils.h"

namespace eicrecon {

void EnergyPositionClusterMerger::process(const Input& input, const Output& output) const {

  const auto [energy_clus, energy_links, pos_clus, pos_links] = input;
  auto [merged_clus, merged_links, merged_assoc]              = output;

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
      const double dphi  = edm4hep::utils::angleAzimuthal(pc.getPosition()) -
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
      new_clus.setTimeError(pc.getTimeError());
      new_clus.setNhits(pc.getNhits() + ec.getNhits());
      new_clus.setPosition(pc.getPosition());
      new_clus.setPositionError(pc.getPositionError());
      new_clus.addToClusters(pc);
      new_clus.addToClusters(ec);
      for (const auto& cl : {pc, ec}) {
        for (const auto& hit : cl.getHits()) {
          new_clus.addToHits(hit);
        }
        new_clus.addToSubdetectorEnergies(cl.getEnergy());
      }

      trace("   --> Found matching energy cluster {}, energy: {}", ec.getObjectID().index,
            ec.getEnergy());
      trace("   --> Created a new combined cluster {}, energy: {}", new_clus.getObjectID().index,
            new_clus.getEnergy());

      // find the first MC link from the energy cluster
      const auto ea = std::ranges::find_if(
          *energy_links, [&ec](const auto& link) { return link.getFrom() == ec; });
      const bool has_ea = ea != energy_links->end();
      // find the first MC link from the position cluster
      const auto pa = std::ranges::find_if(
          *pos_links, [&pc](const auto& link) { return link.getFrom() == pc; });
      const bool has_pa = pa != pos_links->end();
      if (has_ea && has_pa) {
        if ((*pa).getTo() == (*ea).getTo()) {
          // both links agree on the MCParticles entry
          truth::addWeightedRelation(new_clus, (*ea).getTo(), 1.0F, merged_links, merged_assoc);
        } else {
          // both links disagree on the MCParticles entry
          debug("   --> Two associations added to {} and {}", (*ea).getTo().getObjectID().index,
                (*pa).getTo().getObjectID().index);
          truth::addWeightedRelation(new_clus, (*ea).getTo(), 0.5F, merged_links, merged_assoc);
          truth::addWeightedRelation(new_clus, (*pa).getTo(), 0.5F, merged_links, merged_assoc);
        }
      } else if (has_ea) {
        // no position link
        debug("   --> Only added energy cluster association to {}",
              (*ea).getTo().getObjectID().index);
        truth::addWeightedRelation(new_clus, (*ea).getTo(), 1.0F, merged_links, merged_assoc);
      } else if (has_pa) {
        // no energy link
        debug("   --> Only added position cluster association to {}",
              (*pa).getTo().getObjectID().index);
        truth::addWeightedRelation(new_clus, (*pa).getTo(), 1.0F, merged_links, merged_assoc);
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
