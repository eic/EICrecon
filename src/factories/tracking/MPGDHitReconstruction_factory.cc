// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Wouter Deconinck

// Precompile library: explicit instantiation of MPGDHitReconstruction_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "MPGDHitReconstruction_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::MPGDHitReconstruction_factory, eicrecon::MPGDHitReconstructionConfig>;
template class JOmniFactoryGeneratorT<eicrecon::MPGDHitReconstruction_factory>;
