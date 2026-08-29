// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#ifndef EICRECON_FACTORY_PRECOMPILE
// Preprocessor-based precompilation pattern:
// When EICRECON_FACTORY_PRECOMPILE is not defined, plugin code sees only
// forward declarations and extern templates for fast compilation.
// The full definition is compiled once into a precompile library.

namespace eicrecon {
class CalorimeterHitsMerger_factory;
}

extern template class JOmniFactory<eicrecon::CalorimeterHitsMerger_factory, NoConfig>;

#else
// Full factory definition: compiled into precompile library

#include "algorithms/calorimetry/CalorimeterHitsMerger.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class CalorimeterHitsMerger_factory
    : public JOmniFactory<CalorimeterHitsMerger_factory, CalorimeterHitsMergerConfig> {

public:
  using AlgoT = eicrecon::CalorimeterHitsMerger;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::CalorimeterHit> m_hits_input{this};
  PodioOutput<edm4eic::CalorimeterHit> m_hits_output{this};

  ParameterRef<std::string> m_readout{this, "readout", config().readout};
  ParameterRef<std::vector<std::string>> m_field_transformations{this, "fieldTransformations",
                                                                 config().fieldTransformations};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_hits_input()}, {m_hits_output().get()});
  }
};

} // namespace eicrecon

#endif // EICRECON_FACTORY_PRECOMPILE
