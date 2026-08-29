// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2024, Wouter Deconinck, Simon Gardener, Dmitry Kalinkin

// Precompile library: explicit instantiation of ONNXInference_factory
#define EICRECON_FACTORY_PRECOMPILE
#include "ONNXInference_factory.h"

// Explicit template instantiation
template class JOmniFactory<eicrecon::ONNXInference_factory, NoConfig>;
