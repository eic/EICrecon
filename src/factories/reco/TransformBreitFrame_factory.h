// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 John Lajoie

#pragma once

#include <JANA/JEvent.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/reco/TransformBreitFrame.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class TransformBreitFrame_factory
    : public JOmniAlgorithmFactory<TransformBreitFrame_factory, eicrecon::TransformBreitFrame,
                                   NoConfig> {}; // end TransfromBreitFrame_factory definition

} // namespace eicrecon
