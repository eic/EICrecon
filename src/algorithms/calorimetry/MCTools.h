// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim, Sylvester Joosten, Derek Anderson, Wouter Deconinck
#pragma once

//
// @TODO should be migrated to a shared utility function in edm4xxx
//

#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/CaloHitContributionCollection.h>

namespace eicrecon::MCTools {

// Lookup primary MCParticle
// we stop looking if we find the parent has status 1
// so we don't merge e.g. radiative photons with the primary electron as this prevents us
// from properly linking the clusters back to the event geometry. Note that we also
// enforce for this case that no steps back towards the primary were taken to avoid
// storing the first pair of calorimetric showers that start inside the tracking volume.
// Hence, this algorithm will return:
//  - Contribution came from primary: primary
//  - Contribution came from immediate daughter of primary which has no children -> daughter
//  - All other cases (i.e. early showers, multi-radiation): primary
// libraries
inline edm4hep::MCParticle lookup_primary(const edm4hep::CaloHitContribution& contrib) {
  const auto contributor = contrib.getParticle();

  edm4hep::MCParticle primary = contributor;
  size_t steps_taken          = 0; // The number of steps taken looking for the primary
  while (primary.parents_size() > 0) {
    auto parent = primary.getParents(0);
    if (primary.getGeneratorStatus() != 0 ||
        (parent.getGeneratorStatus() != 0 && steps_taken == 0)) {
      break;
    }
    primary = parent;
    steps_taken += 1;
  }
  return primary;
}
} // namespace eicrecon::MCTools
