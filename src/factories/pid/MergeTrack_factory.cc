// Copyright (C) 2026, Wouter Deconinck
// Subject to the terms in the LICENSE file found in the top-level directory.

// Precompile library: explicit instantiation of MergeTrack_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "MergeTrack_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::MergeTrack_factory, eicrecon::NoConfig>;
template class JOmniFactoryGeneratorT<eicrecon::MergeTrack_factory>;
