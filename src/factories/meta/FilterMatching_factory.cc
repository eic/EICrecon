// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Simon Gardner

// Precompile library: explicit instantiation of FilterMatching_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "FilterMatching_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::FilterMatching_factory, NoConfig>;
