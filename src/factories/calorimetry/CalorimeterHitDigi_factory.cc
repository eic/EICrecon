// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2025, Chao Peng, Sylvester Joosten, Whitney Armstrong, Wouter Deconinck, Nathan Brei, Dmitry Kalinkin

// Precompile library: explicit instantiation of CalorimeterHitDigi_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "CalorimeterHitDigi_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::CalorimeterHitDigi_factory, NoConfig>;
