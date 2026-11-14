// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 - 2025 Wouter Deconinck, Dmitry Kalinkin, Derek Anderson

#pragma once

#include "extensions/jana/JOmniAlgorithmFactory.h"
#include "algorithms/reco/ChargedMCParticleSelector.h"

namespace eicrecon {

class ChargedMCParticleSelector_factory
    : public JOmniAlgorithmFactory<ChargedMCParticleSelector_factory,
                                   eicrecon::ChargedMCParticleSelector, NoConfig> {};

} // namespace eicrecon
