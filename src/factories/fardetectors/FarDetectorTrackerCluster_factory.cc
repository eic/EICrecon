// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Wouter Deconinck

// Precompile library: explicit instantiation of FarDetectorTrackerCluster_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "FarDetectorTrackerCluster_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::FarDetectorTrackerCluster_factory, eicrecon::FarDetectorTrackerClusterConfig>;
template class JOmniFactoryGeneratorT<eicrecon::FarDetectorTrackerCluster_factory>;
