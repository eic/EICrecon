// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2024 Wouter Deconinck, Yann Bedfer

// Precompile library: explicit instantiation of MPGDTrackerDigi_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "MPGDTrackerDigi_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::MPGDTrackerDigi_factory, NoConfig>;
