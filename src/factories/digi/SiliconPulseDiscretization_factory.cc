// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Chun Yuen Tsang

// Precompile library: explicit instantiation of SiliconPulseDiscretization_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "SiliconPulseDiscretization_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::SiliconPulseDiscretization_factory, NoConfig>;
