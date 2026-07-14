// Copyright (C) 2022 Sylvester Joosten, Whitney Armstrong, Wouter Deconinck
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "CalorimeterTruthClustering.h"

#include <DD4hep/config.h>
#include <edm4hep/CaloHitContributionCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/RawCalorimeterHit.h>
#include <edm4hep/SimCalorimeterHit.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <cstdint>
#include <gsl/pointers>
#include <map>
#include <vector>

using namespace dd4hep;

namespace eicrecon {

void CalorimeterTruthClustering::init() {}

void CalorimeterTruthClustering::process(const CalorimeterTruthClustering::Input& input,
                                         const CalorimeterTruthClustering::Output& output) const {
  const auto [hits, hitAssociations] = input;
  auto [clusters]                    = output;

  // Map mc track ID to protoCluster index
  std::map<int32_t, int32_t> protoIndex;

  // Loop over all calorimeter hits and sort per mcparticle
  for (const auto& hit : *hits) {
    // Find the associated sim hit(s) for this reconstructed hit using the associations
    for (const auto& assoc : *hitAssociations) {
      // Check if this association corresponds to the current hit
      if (assoc.getRawHit() != hit.getRawHit()) {
        continue;
      }

      // Get the sim hit and its contributions
      const auto& simHit = assoc.getSimHit();

      // Process contributions to find the primary particle
      for (const auto& contrib : simHit.getContributions()) {
        const auto& trackID = contrib.getParticle().getObjectID().index;

        // Create a new protocluster if we don't have one for this trackID
        if (!protoIndex.contains(trackID)) {
          clusters->create();
          protoIndex[trackID] = clusters->size() - 1;
        }

        // Add hit to the appropriate protocluster
        (*clusters)[protoIndex[trackID]].addToHits(hit);
        (*clusters)[protoIndex[trackID]].addToWeights(1);
      }
    }
  }
}

} // namespace eicrecon
