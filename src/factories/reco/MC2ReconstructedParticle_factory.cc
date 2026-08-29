// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

// Precompile library: explicit instantiation of MC2ReconstructedParticle_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "MC2ReconstructedParticle_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::MC2ReconstructedParticle_factory, eicrecon::NoConfig>;
template class JOmniFactoryGeneratorT<eicrecon::MC2ReconstructedParticle_factory>;
