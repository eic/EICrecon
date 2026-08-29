// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Wouter Deconinck

// Precompile library: explicit instantiation of FarDetectorTransportationPostML_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "FarDetectorTransportationPostML_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::FarDetectorTransportationPostML_factory, eicrecon::FarDetectorTransportationPostMLConfig>;
template class JOmniFactoryGeneratorT<eicrecon::FarDetectorTransportationPostML_factory>;
