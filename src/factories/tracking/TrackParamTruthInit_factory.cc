// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

// Precompile library: explicit instantiation of TrackParamTruthInit_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "TrackParamTruthInit_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::TrackParamTruthInit_factory, eicrecon::TrackParamTruthInitConfig>;
template class JOmniFactoryGeneratorT<eicrecon::TrackParamTruthInit_factory>;
