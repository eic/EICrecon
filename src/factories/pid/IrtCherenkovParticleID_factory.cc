// Copyright (C) 2026, Wouter Deconinck
// Subject to the terms in the LICENSE file found in the top-level directory.

// Precompile library: explicit instantiation of IrtCherenkovParticleID_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "IrtCherenkovParticleID_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::IrtCherenkovParticleID_factory, eicrecon::IrtCherenkovParticleIDConfig>;
