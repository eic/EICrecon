// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 EIC-FT

#pragma once

#include "algorithms/digi/RandomNoise.h"
#include "algorithms/digi/RandomNoiseConfig.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"
#include <edm4hep/EventHeader.h>
#include <cstdint>

namespace eicrecon {

// Note: When standalone=true, the algorithm ignores PodioInput and produces a noise-only collection.
// CollectionCollector should be configured to merge this with the primary hit collection later.

/**
 * @brief JANA factory for the RandomNoise algorithm.
 *
 * This factory creates and configures the RandomNoise algorithm,
 * connecting it to the necessary geometry service and managing its
 * input and output data collections.
 */
class RandomNoise_factory : public JOmniFactory<RandomNoise_factory, RandomNoiseConfig> {

public:
  using AlgoT = eicrecon::RandomNoise;

private:
  std::unique_ptr<AlgoT> m_algo;

  // EventHeader input (used only to seed RNG deterministically per event)
  PodioInput<edm4hep::EventHeader> m_in_event_header{this};

  // Output collection of raw tracker hits with noise added.
  PodioOutput<edm4eic::RawTrackerHit> m_out_hits{this};

  // Tunables (forwarded into RandomNoiseConfig)
  // - addNoise: master switch
  ParameterRef<bool> m_addNoise{this, "addNoise", config().addNoise};
  // - n_noise_hits_per_system: Poisson mean
  ParameterRef<int> m_n_noise_hits_per_system{this, "n_noise_hits_per_system",
                                              config().n_noise_hits_per_system};
  // - readout_name: target readout
  ParameterRef<std::string> m_readout_name{this, "readout_name", config().readout_name};
  // Service for accessing detector geometry information.

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(
        config()); // passes ParameterRef values into the algorithm (including standalone mode)
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) override {
    m_algo->process({m_in_event_header()}, {m_out_hits().get()});
  }
};

} // namespace eicrecon
