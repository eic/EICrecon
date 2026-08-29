// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Wouter Deconinck

// Precompile library: explicit instantiation of ScatteredElectronsTruth_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "ScatteredElectronsTruth_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::ScatteredElectronsTruth_factory, eicrecon::NoConfig>;
template class JOmniFactoryGeneratorT<eicrecon::ScatteredElectronsTruth_factory>;
