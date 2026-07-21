// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#pragma once

#include <JANA/JEvent.h>
#include <memory>
#include <string>

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/meta/Cloner.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

/**
 * Factory for cloning collection elements.
 *
 * Template parameter T is the PODIO object type (e.g., edm4eic::Cluster)
 *
 * Example usage in plugin:
 *   app->Add(new JOmniFactoryGeneratorT<Cloner_factory<edm4eic::Cluster>>(
 *       "MyClonedClusters",
 *       {"MySubsetClusters"},
 *       {"MyClonedClusters"},
 *       app
 *   ));
 */
template <typename T> class Cloner_factory : public JOmniFactory<Cloner_factory<T>, NoConfig> {

public:
  using FactoryT = JOmniFactory<Cloner_factory<T>, NoConfig>;
  using AlgoT    = Cloner<T>;

private:
  std::unique_ptr<AlgoT> m_algo;

  typename FactoryT::template PodioInput<T> m_input{this};
  typename FactoryT::template PodioOutput<T> m_output{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(this->logger()->level()));
    m_algo->init();
  }

  void Process(int32_t /*run_number*/, uint64_t /*event_number*/) {
    m_algo->process({m_input()}, {m_output().get()});
  }
};

} // namespace eicrecon
