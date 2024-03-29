// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Simon Gardner

#pragma once

#include "algorithms/meta/FilterByAssociations.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

template <typename FilterByObjectT,auto FilterByMemberFunctionPtr,typename ToFilterObjectT,auto ToFilterMemberFunctionPtr>
class FilterByAssociations_factory : public JOmniFactory<FilterByAssociations_factory<FilterByObjectT,FilterByMemberFunctionPtr,ToFilterObjectT,ToFilterMemberFunctionPtr>> {

  public:
    using AlgoT    = eicrecon::FilterByAssociations<FilterByObjectT,FilterByMemberFunctionPtr,ToFilterObjectT,ToFilterMemberFunctionPtr>;
    using FactoryT = JOmniFactory<FilterByAssociations_factory<FilterByObjectT,FilterByMemberFunctionPtr,ToFilterObjectT,ToFilterMemberFunctionPtr>>;

  private:

    std::unique_ptr<AlgoT> m_algo;

    typename FactoryT::template PodioInput<FilterByObjectT>   m_collection_input {this};
    typename FactoryT::template PodioInput<ToFilterObjectT>  m_associated_input {this};
    typename FactoryT::template PodioOutput<ToFilterObjectT> m_is_associated_output     {this};
    typename FactoryT::template PodioOutput<ToFilterObjectT> m_is_not_associated_output {this};

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
