// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim, Sylvester Joosten, Derek Anderson, Wouter Deconinck

// Precompile library: explicit instantiation of SimCalorimeterHitProcessor_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "SimCalorimeterHitProcessor_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::SimCalorimeterHitProcessor_factory, NoConfig>;
