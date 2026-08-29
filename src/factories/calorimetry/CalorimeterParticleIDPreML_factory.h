// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Dmitry Kalinkin

#pragma once

#ifndef EICRECON_FACTORY_PRECOMPILE
// Preprocessor-based precompilation pattern:
// When EICRECON_FACTORY_PRECOMPILE is not defined, plugin code sees only
// forward declarations and extern templates for fast compilation.
// The full definition is compiled once into a precompile library.

namespace eicrecon {
class CalorimeterParticleIDPreML_factory;
}

extern template class JOmniFactory<eicrecon::CalorimeterParticleIDPreML_factory, NoConfig>;

#else
// Full factory definition: compiled into precompile library

#include "algorithms/onnx/CalorimeterParticleIDPreML.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class CalorimeterParticleIDPreML_factory
    : public JOmniFactory<CalorimeterParticleIDPreML_factory, NoConfig> {

public:
  using AlgoT = eicrecon::CalorimeterParticleIDPreML;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::Cluster> m_cluster_input{this};
  PodioInput<edm4eic::MCRecoClusterParticleAssociation> m_cluster_assoc_input{this};

  PodioOutput<edm4eic::Tensor> m_feature_tensor_output{this};
  PodioOutput<edm4eic::Tensor> m_target_tensor_output{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_cluster_input(), m_cluster_assoc_input()},
                    {m_feature_tensor_output().get(), m_target_tensor_output().get()});
  }
};

} // namespace eicrecon

#endif // EICRECON_FACTORY_PRECOMPILE
