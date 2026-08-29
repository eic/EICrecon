// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 - 2025 Wouter Deconinck, Dmitry Kalinkin, Derek Anderson

// Precompile library: explicit instantiation of ChargedMCParticleSelector_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "ChargedMCParticleSelector_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::ChargedMCParticleSelector_factory, NoConfig>;
