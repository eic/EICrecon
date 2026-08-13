// Copyright (C) 2022 Sylvester Joosten, Whitney Armstrong, Wouter Deconinck
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "CalorimeterTruthClustering.h"

#include <DD4hep/config.h>
#include <edm4hep/CaloHitContributionCollection.h>
#include <edm4hep/RawCalorimeterHit.h>
#include <edm4hep/SimCalorimeterHit.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <cstdint>
#include <gsl/pointers>
#include <map>
#include <set>

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

    std::set<std::size_t> mcIndices;

    // Ignore hit if no associated sim hits
    bool success = false;
    for (const auto& assoc : *hitAssociations) {

      if (assoc.getRawHit() != hit.getRawHit()) {
        continue;
      } else {
        success = true;
        ++nsims;
      }
      const auto& simHit = assoc.getSimHit();

      // Loop through contributions, create a protocluster for each contributing primary
      for (const auto& contrib : simHit.getContributions()) {

        edm4hep::MCParticle primary = get_primary(contrib);
        const auto& trackID         = primary.getObjectID().index;

        // Create a new protocluster if we don't have one for this primary
        if (!protoIndex.contains(trackID)) {
          clusters->create();
          protoIndex[trackID] = clusters->size() - 1;
        }
        mcIndices.insert(trackID);
      }
    }

    // Add hit to the appropriate protoclusters
    if (success) {
      for (const auto& mcIndex : mcIndices) {
        (*clusters)[protoIndex[mcIndex]].addToHits(hit);
        (*clusters)[protoIndex[mcIndex]].addToWeights(1);
      }
    }
  }
}

edm4hep::MCParticle
CalorimeterTruthClustering::get_primary(const edm4hep::CaloHitContribution& contrib) const {
  // get contributing particle
  const auto contributor = contrib.getParticle();

  // walk back through parents to find primary
  //   - TODO finalize primary selection. This
  //     can be improved!!
  edm4hep::MCParticle primary = contributor;
  while (primary.parents_size() > 0) {
    if (primary.getGeneratorStatus() != 0) {
      break;
    }
    primary = primary.getParents(0);
  }
  return primary;
}

} // namespace eicrecon
