// Copyright (C) 2022 Sylvester Joosten, Whitney Armstrong, Wouter Deconinck
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "CalorimeterTruthClustering.h"

#include <DD4hep/config.h>
#include <edm4hep/CaloHitContributionCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <podio/ObjectID.h>
#include <cstddef>
#include <cstdint>
#include <gsl/pointers>
#include <map>

using namespace dd4hep;

namespace eicrecon {

void CalorimeterTruthClustering::init() {}

void CalorimeterTruthClustering::process(const CalorimeterTruthClustering::Input& input,
                                         const CalorimeterTruthClustering::Output& output) const {
  const auto [hits, mc] = input;
  auto [clusters]       = output;

  // Map mc track ID to protoCluster index
  std::map<int32_t, int32_t> protoIndex;

  // Loop over all calorimeter hits and sort per mcparticle
  for (const auto& hit : *hits) {
    // The original algorithm used the following to get the mcHit:
    //
    //    const auto& mcHit     = mc[hit->getObjectID().index];
    //
    // This assumes there is a one-to-one relation between the truth hit
    // (hits) and the reconstructed hit (mc). At least insofar as the
    // index in "hits" is being used to index the "mc" container.
    //
    // If the objects in "hits" have not been added to a collection,
    // then they will have getObjectID().index = "untracked" = -1
    //
    // The way we handle this is here is to check if getObjectID().index
    // is within the size limits of mc which includes >=0. If so, then
    // assume the old code is valid. If not, then we need to search
    // for the right hit.
    // FIXME: This is clearly not the right way to do this! Podio needs
    // FIXME: to be fixed so proper object tracking can be done without
    // FIXME: requiring Collection classes be used to manage all objects.
    std::size_t mcIndex = 0;
    if ((hit.getObjectID().index >= 0) &&
        (hit.getObjectID().index < static_cast<long>(mc->size()))) {
      mcIndex = hit.getObjectID().index;
    } else {
      mcIndex      = 0;
      bool success = false;
      for (auto tmpmc : *mc) {
        if (tmpmc.getCellID() == hit.getCellID()) {
          success = true;
          break;
        }
        mcIndex++;
      }
      if (not success) {
        continue; // ignore hit if we couldn't match it to truth hit
      }
    }

    const auto& trackID = (*mc)[mcIndex].getContributions(0).getParticle().getObjectID().index;
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

} // namespace eicrecon
