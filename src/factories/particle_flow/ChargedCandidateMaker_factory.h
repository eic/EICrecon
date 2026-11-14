// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#pragma once

#include <string>

#include "extensions/jana/JOmniAlgorithmFactory.h"
#include "algorithms/particle_flow/ChargedCandidateMaker.h"

namespace eicrecon {

class ChargedCandidateMaker_factory
    : public JOmniAlgorithmFactory<ChargedCandidateMaker_factory, eicrecon::ChargedCandidateMaker,
                                   NoConfig> {}; // end ChargedCandidateMaker_factory

} // namespace eicrecon
