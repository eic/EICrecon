// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Simon Gardner

#pragma once

#include "algorithms/meta/SubDivideCollection.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

template <class T>
class SubDivideCollection_factory
    : public JOmniFactory<SubDivideCollection_factory<T>, SubDivideCollectionConfig<T>> {

public:
  using AlgoT    = eicrecon::SubDivideCollection<T>;
  using FactoryT = JOmniFactory<SubDivideCollection_factory<T>, SubDivideCollectionConfig<T>>;

private:
  std::unique_ptr<AlgoT> m_algo;

  typename FactoryT::template PodioInput<T> m_input{this};
  typename FactoryT::template VariadicPodioOutput<T> m_split_output{this};

  typename FactoryT::template Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(this->logger()->level()));
    m_algo->applyConfig(this->config());
    m_algo->init();
  }

  void ChangeRun(int32_t /* run_number */) {}

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {

    std::vector<gsl::not_null<typename T::collection_type*>> split_collections;
    for (const auto& split : m_split_output()) {
      split_collections.push_back(gsl::not_null<typename T::collection_type*>(split.get()));
    }
    m_algo->process(m_input(), split_collections);
  };
}; // SplitGeometry_factory
} // namespace eicrecon
