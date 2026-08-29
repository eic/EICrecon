// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Wouter Deconinck

// Precompile library: explicit instantiation of CalorimeterParticleIDPreML_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "CalorimeterParticleIDPreML_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::CalorimeterParticleIDPreML_factory, eicrecon::NoConfig>;
