// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Christopher Dilks

// Merge CherenkovParticleID results from each radiator, for a given Cherenkov PID subsystem

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/CherenkovParticleIDCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// algorithms
#include "algorithms/pid/MergeParticleID.h"
#include "algorithms/pid/MergeParticleIDConfig.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class MergeCherenkovParticleID_factory
    : public JOmniAlgorithmFactory<MergeCherenkovParticleID_factory, eicrecon::MergeParticleID,
                                   MergeParticleIDConfig> {
private:
  // Declare parameters
  ParameterRef<int> m_mergeMode{this, "mergeMode", config().mergeMode};
};
} // namespace eicrecon
