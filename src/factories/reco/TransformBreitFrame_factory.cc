// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 John Lajoie

// Precompile library: explicit instantiation of TransformBreitFrame_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "TransformBreitFrame_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::TransformBreitFrame_factory, NoConfig>;
