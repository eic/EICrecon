// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Wouter Deconinck

// Precompile library: explicit instantiation of SecondaryVerticesHelix_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "SecondaryVerticesHelix_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::SecondaryVerticesHelix_factory, eicrecon::SecondaryVerticesHelixConfig>;
