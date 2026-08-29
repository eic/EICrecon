// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson

// Precompile library: explicit instantiation of CalorimeterClusterShape_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "CalorimeterClusterShape_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::CalorimeterClusterShape_factory, NoConfig>;
