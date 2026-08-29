// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Wouter Deconinck

// Precompile library: explicit instantiation of TrackProjector_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "TrackProjector_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::TrackProjector_factory, NoConfig>;
