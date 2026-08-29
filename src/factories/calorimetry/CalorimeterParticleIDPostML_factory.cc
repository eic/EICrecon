// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Wouter Deconinck

// Precompile library: explicit instantiation of CalorimeterParticleIDPostML_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "CalorimeterParticleIDPostML_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::CalorimeterParticleIDPostML_factory, eicrecon::NoConfig>;
template class JOmniFactoryGeneratorT<eicrecon::CalorimeterParticleIDPostML_factory>;
