// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.

// Precompile library: explicit instantiation of CKFTracking_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "CKFTracking_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::CKFTracking_factory, eicrecon::CKFTrackingConfig>;
