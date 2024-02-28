// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JEvent.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <fmt/core.h>
#include <podio/ObjectID.h>
#include <spdlog/logger.h>
#include <stddef.h>
#include <map>
#include <memory>
#include <ranges>

#include "MatchClusters_factory.h"
