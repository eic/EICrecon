// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright 2024, Dmitry Kalinkin

// Precompile library: explicit instantiation of TracksToParticles_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "TracksToParticles_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::TracksToParticles_factory, NoConfig>;
