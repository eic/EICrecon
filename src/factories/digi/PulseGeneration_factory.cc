// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024-2025 Simon Gardner, Chun Yuen Tsang, Prithwish Tribedy
//                         Minho Kim, Sylvester Joosten, Wouter Deconinck, Dmitry Kalinkin
//

// Precompile library: explicit instantiation of PulseGeneration_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "PulseGeneration_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::PulseGeneration_factory, NoConfig>;
