// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#pragma once

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/meta/CopyAssociations.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

template <class TSource, class TTarget, class TLinked, class TSourceAssoc, class TTargetAssoc = TSourceAssoc>
class CopyAssociations_factory
    : public JOmniFactory<CopyAssociations_factory<TSource, TTarget, TLinked, TSourceAssoc, TTargetAssoc>,
                          CopyAssociationsConfig<TSource, TTarget, TLinked, TSourceAssoc, TTargetAssoc>> {

public:
  using AlgoT    = eicrecon::CopyAssociations<TSource, TTarget, TLinked, TSourceAssoc, TTargetAssoc>;
  using FactoryT = JOmniFactory<CopyAssociations_factory<TSource, TTarget, TLinked, TSourceAssoc, TTargetAssoc>,
                                CopyAssociationsConfig<TSource, TTarget, TLinked, TSourceAssoc, TTargetAssoc>>;

private:
  std::unique_ptr<AlgoT> m_algo;

  typename FactoryT::template PodioInput<TSource> m_in_source{this};
  typename FactoryT::template PodioInput<TTarget> m_in_target{this};
  typename FactoryT::template PodioInput<TSourceAssoc> m_in_source_assoc{this};
  typename FactoryT::template PodioOutput<TTargetAssoc> m_out_target_assoc{this};

  typename FactoryT::template Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(this->logger()->level()));
    m_algo->applyConfig(this->config());
    m_algo->init();
  }

  void Process(int32_t /*run_number*/, uint64_t /*event_number*/) {
    m_algo->process({m_in_source(), m_in_target(), m_in_source_assocs()},
                    {m_out_target_assocs.get()});
  }
};

} // namespace eicrecon
