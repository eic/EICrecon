// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Christopher Dilks

// Merge CherenkovParticleID results from each radiator, for a given Cherenkov PID subsystem

#pragma once

#ifndef EICRECON_FACTORY_PRECOMPILE
// Preprocessor-based precompilation pattern:
// When EICRECON_FACTORY_PRECOMPILE is not defined, plugin code sees only
// forward declarations and extern templates for fast compilation.
// The full definition is compiled once into a precompile library.

namespace eicrecon {
class MergeCherenkovParticleID_factory;
}

extern template class JOmniFactory<eicrecon::MergeCherenkovParticleID_factory, NoConfig>;

#else
// Full factory definition: compiled into precompile library

#include <JANA/JEvent.h>
#include <edm4eic/CherenkovParticleIDCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "algorithms/pid/MergeParticleID.h"
#include "algorithms/pid/MergeParticleIDConfig.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class MergeCherenkovParticleID_factory
    : public JOmniFactory<MergeCherenkovParticleID_factory, MergeParticleIDConfig> {

private:
  // Underlying algorithm
  using AlgoT = eicrecon::MergeParticleID;
  std::unique_ptr<AlgoT> m_algo;

  // Declare inputs
  VariadicPodioInput<edm4eic::CherenkovParticleID> m_particleID_input{this};

  // Declare outputs
  PodioOutput<edm4eic::CherenkovParticleID> m_particleID_output{this};

  // Declare parameters
  ParameterRef<int> m_mergeMode{this, "mergeMode", config().mergeMode};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    auto in1 = m_particleID_input();
    std::vector<gsl::not_null<const edm4eic::CherenkovParticleIDCollection*>> in2;
    std::copy(in1.cbegin(), in1.cend(), std::back_inserter(in2));
    m_algo->process({in2}, {m_particleID_output().get()});
  }
};

} // namespace eicrecon

#endif // EICRECON_FACTORY_PRECOMPILE
