// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Daniel Brandenburg

// Precompile library: explicit instantiation of ScatteredElectronsTruth_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "ScatteredElectronsTruth_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::ScatteredElectronsTruth_factory, NoConfig>;
