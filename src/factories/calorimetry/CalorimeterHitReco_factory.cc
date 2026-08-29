// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Wouter Deconinck

// Precompile library: explicit instantiation of CalorimeterHitReco_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "CalorimeterHitReco_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::CalorimeterHitReco_factory, eicrecon::CalorimeterHitRecoConfig>;
template class JOmniFactoryGeneratorT<eicrecon::CalorimeterHitReco_factory>;
