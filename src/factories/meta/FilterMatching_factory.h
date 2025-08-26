// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Simon Gardner

#pragma once

#include "algorithms/meta/FilterMatching.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

template <typename ToFilterObjectT, auto ToFilterMemberFunctionPtr, typename FilterByObjectT,
          auto FilterByMemberFunctionPtr>
class FilterMatching_factory
    : public JOmniFactory<FilterMatching_factory<ToFilterObjectT, ToFilterMemberFunctionPtr,
                                                 FilterByObjectT, FilterByMemberFunctionPtr>> {

public:
  using AlgoT    = eicrecon::FilterMatching<ToFilterObjectT, ToFilterMemberFunctionPtr,
                                            FilterByObjectT, FilterByMemberFunctionPtr>;
  using FactoryT = JOmniFactory<FilterMatching_factory<ToFilterObjectT, ToFilterMemberFunctionPtr,
                                                       FilterByObjectT, FilterByMemberFunctionPtr>>;

private:
  std::unique_ptr<AlgoT> m_algo;

  typename FactoryT::template PodioInput<ToFilterObjectT> m_collection_input{this};
  typename FactoryT::template PodioInput<FilterByObjectT> m_matched_input{this};
  typename FactoryT::template PodioOutput<ToFilterObjectT> m_is_matched_output{this};
  typename FactoryT::template PodioOutput<ToFilterObjectT> m_is_not_matched_output{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(this->logger()->level()));
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_collection_input(), m_matched_input()},
                    {m_is_matched_output().get(), m_is_not_matched_output().get()});
  };
}; // FilterByAssociations_factory
} // namespace eicrecon
