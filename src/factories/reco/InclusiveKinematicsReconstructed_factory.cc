// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Wouter Deconinck

// Precompile library: explicit instantiation of InclusiveKinematicsReconstructed_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "InclusiveKinematicsReconstructed_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::InclusiveKinematicsReconstructed_factory, NoConfig>;
