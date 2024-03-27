// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Simon Gardner

#pragma once

#include "algorithms/meta/FilterByAssociations.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

template <class T, class TA, auto MemberFunctionPtr>
class FilterByAssociations_factory : public JOmniFactory<FilterByAssociations_factory<T,TA,MemberFunctionPtr>> {

  public:
    using AlgoT    = eicrecon::FilterByAssociations<T,TA,MemberFunctionPtr>;
    using FactoryT = JOmniFactory<FilterByAssociations_factory<T,TA,MemberFunctionPtr>>;

  private:

    std::unique_ptr<AlgoT> m_algo;

    typename FactoryT::template PodioInput<T>   m_collection_input {this};
    typename FactoryT::template PodioInput<TA>  m_associated_input {this};
    typename FactoryT::template PodioOutput<TA> m_is_associated_output     {this};
    typename FactoryT::template PodioOutput<TA> m_is_not_associated_output {this};

  public:
    void Configure() {
      m_algo = std::make_unique<AlgoT>(this->GetPrefix());
      m_algo->init();
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
      m_algo->process({m_collection_input(),m_associated_input()},{m_is_associated_output().get(),m_is_not_associated_output().get()});
    };
}; // FilterByAssociations_factory
} // eicrecon
