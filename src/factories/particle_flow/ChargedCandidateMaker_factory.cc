// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Wouter Deconinck

// Precompile library: explicit instantiation of ChargedCandidateMaker_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "ChargedCandidateMaker_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::ChargedCandidateMaker_factory, eicrecon::NoConfig>;
template class JOmniFactoryGeneratorT<eicrecon::ChargedCandidateMaker_factory>;
