// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Zhongling Ji, Derek Anderson

// Precompile library: explicit instantiation of JetReconstruction_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "JetReconstruction_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::JetReconstruction_factory, NoConfig>;
