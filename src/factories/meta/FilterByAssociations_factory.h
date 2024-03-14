// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Simon Gardner

#pragma once

#include "algorithms/meta/FilterByAssociations.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

template <class T, class TA, auto MemberFunctionPtr>
class FilterByAssociations_factory : public JOmniFactory<FilterByAssociations_factory<T>> {

  public:
    using AlgoT    = eicrecon::FilterByAssociations<T,TA,MemberFunctionPtr>;
    using FactoryT = JOmniFactory<FilterByAssociations_factory<T>>;

  private:

    std::unique_ptr<AlgoT> m_algo;

    typename FactoryT::template PodioInput<T>  m_collection_input {this};
    typename FactoryT::template PodioInput<TA> m_associations_input {this};
    typename FactoryT::template PodioOutput<T> m_filtered_output {this};

  public:
    void Configure() {
      m_algo = std::make_unique<AlgoT>(this->GetPrefix());
      m_algo->init();
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
      m_algo->process({m_collection_input(),m_collection_input()},{m_filtered_output()});
    };
}; // SplitGeometry_factory
} // eicrecon
