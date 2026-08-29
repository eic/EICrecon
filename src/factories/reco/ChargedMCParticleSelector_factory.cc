// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Wouter Deconinck

// Precompile library: explicit instantiation of ChargedMCParticleSelector_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "ChargedMCParticleSelector_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::ChargedMCParticleSelector_factory, eicrecon::NoConfig>;
template class JOmniFactoryGeneratorT<eicrecon::ChargedMCParticleSelector_factory>;
