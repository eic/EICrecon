// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Daniel Brandenburg

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/reco/ScatteredElectronsTruth.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class ScatteredElectronsTruth_factory
    : public JOmniAlgorithmFactory<ScatteredElectronsTruth_factory,
                                   eicrecon::ScatteredElectronsTruth, NoConfig> {};

} // namespace eicrecon
