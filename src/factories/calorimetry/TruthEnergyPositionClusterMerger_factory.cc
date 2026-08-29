// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Wouter Deconinck

// Precompile library: explicit instantiation of TruthEnergyPositionClusterMerger_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "TruthEnergyPositionClusterMerger_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::TruthEnergyPositionClusterMerger_factory, eicrecon::NoConfig>;
template class JOmniFactoryGeneratorT<eicrecon::TruthEnergyPositionClusterMerger_factory>;
