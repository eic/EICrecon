// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tristan Protzman

// Precompile library: explicit instantiation of TrackClusterMatch_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "TrackClusterMatch_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::TrackClusterMatch_factory, NoConfig>;
