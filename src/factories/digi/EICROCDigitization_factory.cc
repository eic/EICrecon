// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Chun Yuen Tsang

// Precompile library: explicit instantiation of EICROCDigitization_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "EICROCDigitization_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::EICROCDigitization_factory, NoConfig>;
