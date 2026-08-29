// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Wouter Deconinck

// Precompile library: explicit instantiation of CaloRemnantCombiner_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "CaloRemnantCombiner_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::CaloRemnantCombiner_factory, eicrecon::CaloRemnantCombinerConfig>;
template class JOmniFactoryGeneratorT<eicrecon::CaloRemnantCombiner_factory>;
