// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Simon Gardner

#pragma once

#include "algorithms/meta/SubDivideCollection.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

template <class T>
class SubDivideCollection_factory : public JOmniFactory<SubDivideCollection_factory<T>, SubDivideCollectionConfig<T>> {

  public:
    using AlgoT = eicrecon::SubDivideCollection<T>;

  private:

    std::unique_ptr<AlgoT> m_algo;

    typename JOmniFactory<SubDivideCollection_factory<T>, SubDivideCollectionConfig<T>>::template PodioInput<T> m_input {this};
    typename JOmniFactory<SubDivideCollection_factory<T>, SubDivideCollectionConfig<T>>::template VariadicPodioOutput<T> m_split_output {this};

    typename JOmniFactory<SubDivideCollection_factory<T>, SubDivideCollectionConfig<T>>::template Service<AlgorithmsInit_service> m_algorithmsInit {this};

public:
    void Configure() {
      m_algo = std::make_unique<AlgoT>(this->GetPrefix());
      m_algo->applyConfig(this->config());
      m_algo->init();
    }

    void setFunctions(std::function<std::vector<int>(const T&)> f) {
      m_algo->setFunction(f);
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {

    std::vector<gsl::not_null<typename T::collection_type*>> split_collections;
    for (const auto& split : m_split_output()) {
      split_collections.push_back(gsl::not_null<typename T::collection_type*>(split.get()));
    }
    m_algo->process(m_input(),split_collections);

};
}; // SplitGeometry_factory
} // eicrecon
