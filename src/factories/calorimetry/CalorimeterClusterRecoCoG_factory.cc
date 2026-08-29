// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2025, Chao Peng, Sylvester Joosten, Whitney Armstrong, Wouter Deconinck, Nathan Brei, Sebouh Paul, Dmitry Kalinkin, Derek Anderson

// Precompile library: explicit instantiation of CalorimeterClusterRecoCoG_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "CalorimeterClusterRecoCoG_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::CalorimeterClusterRecoCoG_factory, NoConfig>;
