// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Wouter Deconinck

// Precompile library: explicit instantiation of FarDetectorLinearTracking_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "FarDetectorLinearTracking_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::FarDetectorLinearTracking_factory, NoConfig>;
