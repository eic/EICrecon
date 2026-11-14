// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/reco/MC2ReconstructedParticle.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class MC2ReconstructedParticle_factory
    : public JOmniAlgorithmFactory<MC2ReconstructedParticle_factory,
                                   eicrecon::MC2ReconstructedParticle, NoConfig> {};

} // namespace eicrecon
