// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

// Precompile library: explicit instantiation of TrackClusterSubtractor_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "TrackClusterSubtractor_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::TrackClusterSubtractor_factory, NoConfig>;
