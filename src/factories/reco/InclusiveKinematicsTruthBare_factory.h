// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Wouter Deconinck

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/reco/InclusiveKinematicsTruthBare.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class InclusiveKinematicsTruthBare_factory
    : public JOmniAlgorithmFactory<InclusiveKinematicsTruthBare_factory,
                                   eicrecon::InclusiveKinematicsTruthBare, NoConfig> {};

} // namespace eicrecon
