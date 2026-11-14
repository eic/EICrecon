// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, ePIC Collaboration

#pragma once

#include "algorithms/meta/SortSubsetCollection.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"
#include <cstdint>
#include <memory>

namespace eicrecon {

template <class T, auto Accessor>
class SortSubsetCollection_factory
    : public JOmniAlgorithmFactory<SortSubsetCollection_factory<T, Accessor>,
                                   SortSubsetCollection<T, decltype(Accessor)>> {
public:
  // The algorithm takes the sort accessor as a constructor argument
  void Configure() {
    this->m_algo =
        std::make_unique<SortSubsetCollection<T, decltype(Accessor)>>(this->GetPrefix(), Accessor);
    this->m_algo->level(static_cast<algorithms::LogLevel>(this->logger()->level()));
    this->m_algo->applyConfig(this->config());
    this->m_algo->init();
  }
};

} // namespace eicrecon
