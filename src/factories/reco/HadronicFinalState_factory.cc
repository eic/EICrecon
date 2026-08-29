// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Tyler Kutz

// Precompile library: explicit instantiation of HadronicFinalState_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "HadronicFinalState_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::HadronicFinalState_factory, NoConfig>;
