// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

// Precompile library: explicit instantiation of EnergyPositionClusterMerger_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "EnergyPositionClusterMerger_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::EnergyPositionClusterMerger_factory, NoConfig>;
