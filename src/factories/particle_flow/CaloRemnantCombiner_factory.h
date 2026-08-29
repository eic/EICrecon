// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Subhadip Pal

#pragma once
#include "algorithms/particle_flow/CaloRemnantCombinerConfig.h"
#include "extensions/jana/JOmniFactory.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#ifndef EICRECON_FACTORY_PRECOMPILE

namespace eicrecon {
class CaloRemnantCombiner_factory;
}

extern template class JOmniFactory<eicrecon::CaloRemnantCombiner_factory, eicrecon::CaloRemnantCombinerConfig>;
extern template class JOmniFactoryGeneratorT<eicrecon::CaloRemnantCombiner_factory>;

#else

#include "algorithms/particle_flow/CaloRemnantCombiner.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon {

class CaloRemnantCombiner_factory
    : public JOmniFactory<CaloRemnantCombiner_factory, CaloRemnantCombinerConfig> {

public:
  using AlgoT = eicrecon::CaloRemnantCombiner;

private:
  // Underlying algorithm
  std::unique_ptr<AlgoT> m_algo;

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->init();
  }

  void ChangeRun(int64_t run_number) {
  }

  void Process(const CaloRemnantCombiner_factory::InputT<0>& input_truth_mc_particles,
               const CaloRemnantCombiner_factory::InputT<1>& input_rec_particles,
               const CaloRemnantCombiner_factory::OutputT<0>& output_remnant_particles) const {
    m_algo->process({input_truth_mc_particles, input_rec_particles}, {output_remnant_particles});
  }
};

}  // namespace eicrecon

#endif
