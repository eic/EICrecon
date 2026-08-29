// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Wouter Deconinck

// Precompile library: explicit instantiation of ActsTrackMerger_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "ActsTrackMerger_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::ActsTrackMerger_factory, eicrecon::NoConfig>;
template class JOmniFactoryGeneratorT<eicrecon::ActsTrackMerger_factory>;
