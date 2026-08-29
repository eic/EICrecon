// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Wouter Deconinck

// Precompile library: explicit instantiation of ChargedReconstructedParticleSelector_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "ChargedReconstructedParticleSelector_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::ChargedReconstructedParticleSelector_factory, eicrecon::NoConfig>;
template class JOmniFactoryGeneratorT<eicrecon::ChargedReconstructedParticleSelector_factory>;
