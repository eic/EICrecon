// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <algorithms/logger.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <spdlog/logger.h>
#include <stdint.h>
#include <memory>

#include "algorithms/reco/MatchClusters.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class MatchClusters_factory
    : public JOmniAlgorithmFactory<MatchClusters_factory, eicrecon::MatchClusters, NoConfig> {};

} // namespace eicrecon
