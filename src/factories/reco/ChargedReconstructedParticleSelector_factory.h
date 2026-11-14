// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 - 2025 Derek Anderson, Wouter Deconinck

#pragma once

#include "extensions/jana/JOmniAlgorithmFactory.h"
#include "algorithms/reco/ChargedReconstructedParticleSelector.h"

namespace eicrecon {

class ChargedReconstructedParticleSelector_factory
    : public JOmniAlgorithmFactory<ChargedReconstructedParticleSelector_factory,
                                   eicrecon::ChargedReconstructedParticleSelector, NoConfig> {};

} // namespace eicrecon
