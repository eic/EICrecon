// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Wouter Deconinck

// Precompile library: explicit instantiation of MatchClusters_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "MatchClusters_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::MatchClusters_factory, eicrecon::NoConfig>;
template class JOmniFactoryGeneratorT<eicrecon::MatchClusters_factory>;
