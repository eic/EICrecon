// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#pragma once

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/meta/CopyLinks.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

template <class TSource, class TTarget, class TSourceLink, class TTargetLink = TSourceLink>
class CopyLinks_factory
    : public JOmniFactory<CopyLinks_factory<TSource, TTarget, TSourceLink, TTargetLink>, NoConfig> {

public:
  using AlgoT    = eicrecon::CopyLinks<TSource, TTarget, TSourceLink, TTargetLink>;
  using FactoryT = JOmniFactory<CopyLinks_factory<TSource, TTarget, TSourceLink, TTargetLink>, NoConfig>;

private:
  std::unique_ptr<AlgoT> m_algo;

  typename FactoryT::template PodioInput<TSource> m_in_source{this};
  typename FactoryT::template PodioInput<TTarget> m_in_target{this};
  typename FactoryT::template PodioInput<TSourceLink> m_in_source_link{this};
  typename FactoryT::template PodioOutput<TTargetLink> m_out_target_link{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(this->logger()->level()));
    m_algo->init();
  }

  void Process(int32_t /*run_number*/, uint64_t /*event_number*/) {
    m_algo->process({m_in_source(), m_in_target(), m_in_source_links},
                    {m_out_target_links.get()});
  }
};

} // namespace eicrecon
