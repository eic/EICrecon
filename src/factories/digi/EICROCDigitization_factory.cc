// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Wouter Deconinck

// Precompile library: explicit instantiation of EICROCDigitization_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "EICROCDigitization_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::EICROCDigitization_factory, eicrecon::EICROCDigitizationConfig>;
template class JOmniFactoryGeneratorT<eicrecon::EICROCDigitization_factory>;
