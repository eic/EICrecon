// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Wouter Deconinck

// Precompile library: explicit instantiation of TrackSeeding_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "TrackSeeding_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::TrackSeeding_factory, eicrecon::TrackSeedingConfig>;
template class JOmniFactoryGeneratorT<eicrecon::TrackSeeding_factory>;
