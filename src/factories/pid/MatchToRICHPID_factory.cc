// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright 2024, Dmitry Kalinkin

// Precompile library: explicit instantiation of MatchToRICHPID_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "MatchToRICHPID_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::MatchToRICHPID_factory, NoConfig>;
