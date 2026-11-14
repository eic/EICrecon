// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Simon Gardner

#pragma once

#include "extensions/jana/JOmniAlgorithmFactory.h"
#include "algorithms/meta/CollectionCollector.h"

namespace eicrecon {

template <class T, bool IsOptional = false>
class CollectionCollector_factory
    : public JOmniAlgorithmFactory<CollectionCollector_factory<T, IsOptional>,
                                   CollectionCollector<typename T::collection_type>, NoConfig,
                                   /* OptionalVariadicInputs = */ IsOptional> {};

} // namespace eicrecon
