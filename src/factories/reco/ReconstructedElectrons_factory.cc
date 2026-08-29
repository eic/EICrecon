// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Daniel Brandenburg, Wouter Deconinck

// Precompile library: explicit instantiation of ReconstructedElectrons_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "ReconstructedElectrons_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::ReconstructedElectrons_factory, NoConfig>;
