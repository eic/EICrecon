// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Wouter Deconinck
//                         Minho Kim, Sylvester Joosten, Wouter Deconinck, Dmitry Kalinkin
//

// Precompile library: explicit instantiation of PulseGeneration_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "PulseGeneration_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::PulseGeneration_factory, NoConfig>;
