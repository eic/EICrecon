// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Xin Dong

// Precompile library: explicit instantiation of PrimaryVertices_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "PrimaryVertices_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::PrimaryVertices_factory, NoConfig>;
