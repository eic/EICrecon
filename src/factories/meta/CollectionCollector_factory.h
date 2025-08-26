// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Simon Gardner

#include "extensions/jana/JOmniFactory.h"
#include "algorithms/meta/CollectionCollector.h"

namespace eicrecon {

template <class T, bool IsOptional = false>
class CollectionCollector_factory
    : public JOmniFactory<CollectionCollector_factory<T, IsOptional>> {
public:
  using AlgoT = eicrecon::CollectionCollector<typename T::collection_type>;

private:
  std::unique_ptr<AlgoT> m_algo;

  typename JOmniFactory<CollectionCollector_factory<T, IsOptional>>::template VariadicPodioInput<
      T, IsOptional>
      m_inputs{this};
  typename JOmniFactory<CollectionCollector_factory<T, IsOptional>>::template PodioOutput<T>
      m_output{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(this->logger()->level()));
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    std::vector<gsl::not_null<const typename T::collection_type*>> in_collections;
    for (const auto& in_collection : m_inputs()) {
      in_collections.push_back(gsl::not_null<const typename T::collection_type*>{in_collection});
    }
    typename T::collection_type* merged_collection = m_output().get();
    m_algo->process(in_collections, merged_collection);
  };
};

} // namespace eicrecon
