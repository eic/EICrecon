// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, ePIC Collaboration

#pragma once

#ifndef EICRECON_FACTORY_PRECOMPILE

namespace eicrecon {
class SortSubsetCollection_factory;
}

extern template class JOmniFactory<eicrecon::SortSubsetCollection_factory, NoConfig>;

#else

#include "algorithms/meta/SortSubsetCollection.h"
#include "extensions/jana/JOmniFactory.h"
#include <cstdint>
#include <memory>

namespace eicrecon {

class SortSubsetCollection_factory
    : public JOmniFactory<SortSubsetCollection_factory<T, Accessor>, NoConfig> {

public:
  using AlgoT = eicrecon::SortSubsetCollection<T, decltype(Accessor)>;

private:
  std::unique_ptr<AlgoT> m_algo;

  typename JOmniFactory<SortSubsetCollection_factory<T, Accessor>, NoConfig>::template PodioInput<T>
      m_input{this};
  typename JOmniFactory<SortSubsetCollection_factory<T, Accessor>,
                        NoConfig>::template PodioOutput<T>
      m_output{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix(), Accessor);
    m_algo->level(static_cast<algorithms::LogLevel>(this->logger()->level()));
    m_algo->applyConfig(this->config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_input()}, {m_output().get()});
  };
};

} // namespace eicrecon

#endif // EICRECON_FACTORY_PRECOMPILE
