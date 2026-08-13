// SPDX-License-Identifier: LGPL-3.0-or-later
// codCopyright (C) 2026 Takuya Kumaoka

#pragma once

#include <cstddef>
#include <cstdint>
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
