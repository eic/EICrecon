// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Sylvester Joosten, Whitney Armstrong, Wouter Deconinck, Derek Anderson

#include "CalorimeterTruthClustering.h"

#include <DD4hep/config.h>
#include <edm4hep/RawCalorimeterHit.h>
#include <edm4hep/SimCalorimeterHit.h>
#include <podio/LinkNavigator.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <cstddef>
#include <cstdint>
#include <map>
#include <set>
#include <tuple>
#include <vector>

using namespace dd4hep;

namespace eicrecon {

void CalorimeterTruthClustering::init() {}

void CalorimeterTruthClustering::process(const CalorimeterTruthClustering::Input& input,
                                         const CalorimeterTruthClustering::Output& output) const {

  const auto [hits, hitLinks] = input;
  auto [clusters]             = output;

  const auto navigator = podio::LinkNavigator(*hitLinks);

  // Map mc track ID to protoCluster index
  std::map<int32_t, int32_t> protoIndex;

  // Loop over all calorimeter hits and sort per mcparticle
  for (const auto& hit : *hits) {

    const auto linkedSimHits = navigator.getLinked(hit.getRawHit());

    // Ignore hit if no associated sim hits
    std::set<std::size_t> mcIndices;
    for (const auto& [simHit, weight] : linkedSimHits) {

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
      for (const auto& mcIndex : mcIndices) {
        (*clusters)[protoIndex[mcIndex]].addToHits(hit);
        (*clusters)[protoIndex[mcIndex]].addToWeights(1);
}
  }
}

edm4hep::MCParticle
CalorimeterTruthClustering::get_primary(const edm4hep::CaloHitContribution& contrib) {
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
