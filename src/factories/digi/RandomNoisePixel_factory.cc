// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minjung Kim, Joshua Sobaljic, Shujie Li

// Precompile library: explicit instantiation of RandomNoisePixel_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "RandomNoisePixel_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::RandomNoisePixel_factory, NoConfig>;
