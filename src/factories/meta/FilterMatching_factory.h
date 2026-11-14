// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Simon Gardner

#pragma once

#include "algorithms/meta/FilterMatching.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

template <typename ToFilterObjectT, auto ToFilterMemberFunctionPtr, typename FilterByObjectT,
          auto FilterByMemberFunctionPtr>
class FilterMatching_factory
    : public JOmniAlgorithmFactory<
          FilterMatching_factory<ToFilterObjectT, ToFilterMemberFunctionPtr, FilterByObjectT,
                                 FilterByMemberFunctionPtr>,
          FilterMatching<ToFilterObjectT, ToFilterMemberFunctionPtr, FilterByObjectT,
                         FilterByMemberFunctionPtr>> {}; // FilterByAssociations_factory

} // namespace eicrecon
