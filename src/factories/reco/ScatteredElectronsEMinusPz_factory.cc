// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Wouter Deconinck

// Precompile library: explicit instantiation of ScatteredElectronsEMinusPz_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "ScatteredElectronsEMinusPz_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::ScatteredElectronsEMinusPz_factory, eicrecon::ScatteredElectronsEMinusPzConfig>;
template class JOmniFactoryGeneratorT<eicrecon::ScatteredElectronsEMinusPz_factory>;
