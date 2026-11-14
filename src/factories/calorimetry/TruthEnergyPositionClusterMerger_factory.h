// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include <edm4eic/EDM4eicVersion.h>
#include "algorithms/calorimetry/TruthEnergyPositionClusterMerger.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class TruthEnergyPositionClusterMerger_factory
    : public JOmniAlgorithmFactory<TruthEnergyPositionClusterMerger_factory,
                                   eicrecon::TruthEnergyPositionClusterMerger, NoConfig> {};

} // namespace eicrecon
