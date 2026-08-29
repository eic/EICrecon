// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Wouter Deconinck

// Precompile library: explicit instantiation of SecondaryVertexFinder_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "SecondaryVertexFinder_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::SecondaryVertexFinder_factory, eicrecon::SecondaryVertexFinderConfig>;
template class JOmniFactoryGeneratorT<eicrecon::SecondaryVertexFinder_factory>;
