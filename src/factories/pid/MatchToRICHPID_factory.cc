// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Wouter Deconinck

// Precompile library: explicit instantiation of MatchToRICHPID_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "MatchToRICHPID_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::MatchToRICHPID_factory, eicrecon::MatchToRICHPIDConfig>;
template class JOmniFactoryGeneratorT<eicrecon::MatchToRICHPID_factory>;
