// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Wouter Deconinck
//

// Precompile library: explicit instantiation of UndoAfterBurnerMCParticles_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "UndoAfterBurnerMCParticles_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::UndoAfterBurnerMCParticles_factory, eicrecon::UndoAfterBurnerConfig>;
