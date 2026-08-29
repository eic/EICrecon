// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Nathan Brei

// Precompile library: explicit instantiation of PIDLookup_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "PIDLookup_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::PIDLookup_factory, NoConfig>;
