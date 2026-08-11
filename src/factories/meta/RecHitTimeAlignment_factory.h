// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2024 Whitney Armstrong, Wouter Deconinck, Dmitry Romanov

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include "algorithms/meta/RecHitTimeAlignment.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

template <typename HitT>
class RecHitTimeAlignment_factory
    : public JOmniFactory<RecHitTimeAlignment_factory<HitT>, RecHitTimeAlignmentConfig> {
public:
  using FactoryT = JOmniFactory<RecHitTimeAlignment_factory<HitT>, RecHitTimeAlignmentConfig>;
  using AlgoT    = RecHitTimeAlignment<HitT>;

private:
  std::unique_ptr<AlgoT> m_algo;

  typename FactoryT::template VariadicPodioInput<HitT, true> m_hits_in{this};
  typename FactoryT::template VariadicPodioOutput<HitT> m_hits_out{this};

  typename FactoryT::template ParameterRef<double> m_reference_inverse_velocity{
      this, "referenceInverseVelocity", this->config().reference_inverse_velocity};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(this->logger()->level()));
    m_algo->applyConfig(this->config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    for (std::size_t index = 0; index < m_hits_in().size(); ++index) {
      const auto* hits_in = m_hits_in().at(index);
      if (hits_in != nullptr) {
        m_algo->process({hits_in}, {m_hits_out().at(index).get()});
      }
    }
  }
};

} // namespace eicrecon
