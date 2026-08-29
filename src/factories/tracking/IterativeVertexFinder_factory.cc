// Created by Joe Osborn
// Subject to the terms in the LICENSE file found in the top-level directory.

// Precompile library: explicit instantiation of IterativeVertexFinder_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "IterativeVertexFinder_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::IterativeVertexFinder_factory,
                            eicrecon::IterativeVertexFinderConfig>;
