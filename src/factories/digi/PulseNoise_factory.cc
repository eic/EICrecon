// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Wouter Deconinck

// Precompile library: explicit instantiation of PulseNoise_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "PulseNoise_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::PulseNoise_factory, eicrecon::PulseNoiseConfig>;
template class JOmniFactoryGeneratorT<eicrecon::PulseNoise_factory>;
