// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim, Sylvester Joosten, Derek Anderson

#pragma once

#include "algorithms/calorimetry/SimCalorimeterHitProcessor.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class SimCalorimeterHitProcessor_factory
    : public JOmniFactory<SimCalorimeterHitProcessor_factory, SimCalorimeterHitProcessorConfig> {

public:
  using AlgoT = eicrecon::SimCalorimeterHitProcessor;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4hep::SimCalorimeterHit> m_hits_input{this};
  PodioOutput<edm4hep::SimCalorimeterHit> m_hits_output{this};
  PodioOutput<edm4hep::CaloHitContribution> m_hits_contribs_output{this};

  ParameterRef<std::vector<double>> m_attenuationParameters{this, "attenuationParameters",
                                                            config().attPars};
  ParameterRef<std::string> m_attenuationField{this, "attenuationField", config().attenuationField};
  ParameterRef<std::string> m_mergeField{this, "mergeField", config().mergeField};
  ParameterRef<std::string> m_readout{this, "readout", config().readout};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void ChangeRun(int32_t /* run_number */) {}

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_hits_input()}, {m_hits_output().get(), m_hits_contribs_output().get()});
  }
};

} // namespace eicrecon
