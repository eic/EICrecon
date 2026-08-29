// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Wouter Deconinck

// Precompile library: explicit instantiation of LGADHitClustering_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "LGADHitClustering_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::LGADHitClustering_factory, eicrecon::LGADHitClusteringConfig>;
template class JOmniFactoryGeneratorT<eicrecon::LGADHitClustering_factory>;
