// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Sebouh Paul

// Precompile library: explicit instantiation of HEXPLIT_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "HEXPLIT_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::HEXPLIT_factory, NoConfig>;
