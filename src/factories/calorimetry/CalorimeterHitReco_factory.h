// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include "algorithms/calorimetry/CalorimeterHitReco.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class CalorimeterHitReco_factory
    : public JOmniFactory<CalorimeterHitReco_factory, CalorimeterHitRecoConfig> {

private:
public:
  using AlgoT = eicrecon::CalorimeterHitReco;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4hep::RawCalorimeterHit> m_raw_hits_input{this};
  PodioOutput<edm4eic::CalorimeterHit> m_rec_hits_output{this};

  ParameterRef<unsigned int> m_capADC{this, "capacityADC", config().capADC};
  ParameterRef<double> m_dyRangeADC{this, "dynamicRangeADC", config().dyRangeADC};
  ParameterRef<unsigned int> m_pedMeanADC{this, "pedestalMean", config().pedMeanADC};
  ParameterRef<double> m_pedSigmaADC{this, "pedestalSigma", config().pedSigmaADC};
  ParameterRef<double> m_resolutionTDC{this, "resolutionTDC", config().resolutionTDC};
  ParameterRef<double> m_thresholdFactor{this, "thresholdFactor", config().thresholdFactor};
  ParameterRef<double> m_thresholdValue{this, "thresholdValue", config().thresholdValue};
  ParameterRef<std::string> m_samplingFraction{this, "samplingFraction", config().sampFrac};
  ParameterRef<std::string> m_readout{this, "readout", config().readout};
  ParameterRef<std::string> m_layerField{this, "layerField", config().layerField};
  ParameterRef<std::string> m_sectorField{this, "sectorField", config().sectorField};
  ParameterRef<std::string> m_localDetElement{this, "localDetElement", config().localDetElement};
  ParameterRef<std::vector<std::string>> m_localDetFields{this, "localDetFields",
                                                          config().localDetFields};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_raw_hits_input()}, {m_rec_hits_output().get()});
  }
};

} // namespace eicrecon
