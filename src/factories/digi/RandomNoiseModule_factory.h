// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minjung Kim, Joshua Sobaljic, Shujie Li

#pragma once

#include "algorithms/digi/RandomNoiseModule.h"
#include "algorithms/digi/RandomNoiseModuleConfig.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"
#include <edm4hep/EventHeader.h>
#include <cstdint>

namespace eicrecon {

/**
 * @brief JANA factory for the RandomNoiseModule algorithm.
 *
 * This factory creates and configures the RandomNoiseModule algorithm,
 * connecting it to the necessary geometry service and managing its
 * input and output data collections.
 */
class RandomNoiseModule_factory
    : public JOmniFactory<RandomNoiseModule_factory, RandomNoiseModuleConfig> {

public:
  using AlgoT = eicrecon::RandomNoiseModule;

private:
  std::unique_ptr<AlgoT> m_algo;

  // EventHeader input (used only to seed RNG deterministically per event)
  PodioInput<edm4hep::EventHeader> m_in_event_header{this};

  // Noise-only raw tracker hits. CollectionCollector can merge these later.
  PodioOutput<edm4eic::RawTrackerHit> m_out_hits{this};

  // Tunables (forwarded into RandomNoiseModuleConfig)
  // - addNoise: master switch
  ParameterRef<bool> m_addNoise{this, "addNoise", config().addNoise};
  // - n_noise_hits_per_system: Poisson mean
  ParameterRef<int> m_n_noise_hits_per_system{this, "n_noise_hits_per_system",
                                              config().n_noise_hits_per_system};
  // - readout_name: target readout
  ParameterRef<std::string> m_readout_name{this, "readout_name", config().readout_name};
  // - layer_id / n_noise_hits_per_layer / detector_names: optional per-layer overrides
  ParameterRef<std::vector<int>> m_layer_id{this, "layer_id", config().layer_id};
  ParameterRef<std::vector<int>> m_n_noise_hits_per_layer{this, "n_noise_hits_per_layer",
                                                          config().n_noise_hits_per_layer};
  ParameterRef<std::vector<std::string>> m_detector_names{this, "detector_names",
                                                          config().detector_names};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) override {
    m_algo->process({m_in_event_header()}, {m_out_hits().get()});
  }
};

} // namespace eicrecon
