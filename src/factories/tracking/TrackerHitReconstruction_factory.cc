// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Wouter Deconinck

// Precompile library: explicit instantiation of TrackerHitReconstruction_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "TrackerHitReconstruction_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::TrackerHitReconstruction_factory, eicrecon::TrackerHitReconstructionConfig>;
template class JOmniFactoryGeneratorT<eicrecon::TrackerHitReconstruction_factory>;
