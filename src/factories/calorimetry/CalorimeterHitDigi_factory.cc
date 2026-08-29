// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Wouter Deconinck

// Precompile library: explicit instantiation of CalorimeterHitDigi_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "CalorimeterHitDigi_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::CalorimeterHitDigi_factory, NoConfig>;
