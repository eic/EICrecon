// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim, Sylvester Joosten, Derek Anderson, Wouter Deconinck

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
                                                            config().attenuationParameters};
  ParameterRef<std::string> m_attenuationReferencePositionName{
      this, "attenuationReferencePositionName", config().attenuationReferencePositionName};
  ParameterRef<std::vector<std::string>> m_hitMergeFields{this, "hitMergeFields",
                                                          config().hitMergeFields};
  ParameterRef<std::vector<std::string>> m_contributionMergeFields{
      this, "contributionMergeFields", config().contributionMergeFields};
  ParameterRef<std::string> m_readout{this, "readout", config().readout};
  ParameterRef<double> m_inversePropagationSpeed{this, "inversePropagationSpeed",
                                                 config().inversePropagationSpeed};
  ParameterRef<double> m_fixedTimeDelay{this, "fixedTimeDelay", config().fixedTimeDelay};
  ParameterRef<double> m_timeWindow{this, "timeWindow", config().timeWindow};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_hits_input()}, {m_hits_output().get(), m_hits_contribs_output().get()});
  }
};

} // namespace eicrecon
