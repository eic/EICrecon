// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Wouter Deconinck

// Merge CherenkovParticleID results from each radiator, for a given Cherenkov PID subsystem

// Precompile library: explicit instantiation of MergeCherenkovParticleID_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "MergeCherenkovParticleID_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::MergeCherenkovParticleID_factory, NoConfig>;
