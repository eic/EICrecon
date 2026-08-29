// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Wouter Deconinck

// Precompile library: explicit instantiation of SiliconChargeSharing_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "SiliconChargeSharing_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::SiliconChargeSharing_factory, eicrecon::SiliconChargeSharingConfig>;
template class JOmniFactoryGeneratorT<eicrecon::SiliconChargeSharing_factory>;
