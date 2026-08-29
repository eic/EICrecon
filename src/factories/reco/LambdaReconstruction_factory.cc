// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Sebouh Paul, Baptiste Fraisse

// Precompile library: explicit instantiation of LambdaReconstruction_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "LambdaReconstruction_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::LambdaReconstruction_factory, NoConfig>;
