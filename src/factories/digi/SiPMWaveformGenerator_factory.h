// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Derek Anderson

#pragma once

// eicrecon components
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "algorithms/digi/SiPMWaveformGenerator.h"

namespace eicrecon {

class SiPMWaveformGenerator_factory
    : public JOmniFactory<SiPMWaveformGenerator_factory, SiPMWaveformGeneratorConfig> {

public:
  using AlgoT = eicrecon::SiPMWaveformGenerator;

private:
  // algorithm to run
  std::unique_ptr<AlgoT> m_algo;

  // input collections
  PodioInput<edm4hep::SimCalorimeterHit> m_simcalohit_input{this};

  // output collections
  PodioOutput<edm4hep::RawTimeSeries> m_rawtimeseries_output{this};

  // parameters
  ParameterRef<size_t> m_nSamples{this, "nSamples", config().nSamples};

  // services
  Service<AlgorithmsInit_service> m_algoInitSvc{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void ChangeRun(int64_t run_number) {
    //... nothing to do here ...//
  }

  void Process(int64_t run_number, uint64_t event_number) {
    m_algo->process({m_simcalohit_input()}, {m_rawtimeseries_output().get()});
  }

}; // end SiPMWaveformGenerator_factory

} // namespace eicrecon
