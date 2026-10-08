// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#pragma once

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/meta/CopyLinks.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

template <class SourceT, class SourceLinkT, class TargetT = SourceT,
          class TargetLinkT = SourceLinkT>
class CopyLinks_factory
    : public JOmniFactory<CopyLinks_factory<SourceT, SourceLinkT, TargetT, TargetLinkT>, NoConfig> {

public:
  using AlgoT = eicrecon::CopyLinks<SourceT, SourceLinkT, TargetT, TargetLinkT>;
  using FactoryT =
      JOmniFactory<CopyLinks_factory<SourceT, SourceLinkT, TargetT, TargetLinkT>, NoConfig>;

private:
  std::unique_ptr<AlgoT> m_algo;

  typename FactoryT::template PodioInput<SourceT> m_in_source{this};
  typename FactoryT::template PodioInput<TargetT> m_in_target{this};
  typename FactoryT::template PodioInput<SourceLinkT> m_in_source_link{this};
  typename FactoryT::template PodioOutput<TargetLinkT> m_out_target_link{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(this->logger()->level()));
    m_algo->init();
  }

  void Process(int32_t /*run_number*/, uint64_t /*event_number*/) {
    m_algo->process({m_in_source(), m_in_target(), m_in_source_links()},
                    {m_out_target_links.get()});
  }
};

} // namespace eicrecon
