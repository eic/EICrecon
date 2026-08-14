// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Takuya Kumaoka

#pragma once

#include <memory>

#include "algorithms/event_building/HitTimeAlignment.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

template <typename HitT>
class HitTimeAlignment_factory
    : public JOmniFactory<HitTimeAlignment_factory<HitT>, HitTimeAlignmentConfig> {
public:
  using FactoryT = JOmniFactory<HitTimeAlignment_factory<HitT>, HitTimeAlignmentConfig>;
  using AlgoT    = HitTimeAlignment<HitT>;

private:
  std::unique_ptr<AlgoT> m_algo;

  typename FactoryT::template PodioInput<HitT, true> m_hits_in{this};
  typename FactoryT::template PodioOutput<HitT> m_hits_out{this};

  typename FactoryT::template ParameterRef<double> m_refInverseVelocity{
      this, "referenceInverseVelocity", this->config().refInverseVelocity};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(this->logger()->level()));
    m_algo->applyConfig(this->config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    const auto* hits_in = m_hits_in();

    if (hits_in != nullptr) {
      m_algo->process({hits_in}, {m_hits_out().get()});
    }
  }
};

} // namespace eicrecon