// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Minjung Kim, Barak Schmookler
// Precompile library: explicit instantiation of AmbiguitySolver_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "AmbiguitySolver_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::AmbiguitySolver_factory, NoConfig>;
