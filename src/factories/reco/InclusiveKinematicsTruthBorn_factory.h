// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/reco/InclusiveKinematicsTruthBorn.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class InclusiveKinematicsTruthBorn_factory
    : public JOmniAlgorithmFactory<InclusiveKinematicsTruthBorn_factory,
                                   eicrecon::InclusiveKinematicsTruthBorn, NoConfig> {};

} // namespace eicrecon
