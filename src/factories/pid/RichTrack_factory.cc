// Copyright (C) 2026, Wouter Deconinck
// Subject to the terms in the LICENSE file found in the top-level directory.

// Precompile library: explicit instantiation of RichTrack_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "RichTrack_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::RichTrack_factory, NoConfig>;
